import json
import info
import os
import time
import threading
from multiprocessing import Queue


q = Queue()


def search(s, line):
    s.send(json.dumps({'cmd': 'search'}).encode('utf-8'))
    data = json.loads(s.recv(1024).decode('utf-8'))
    print(info.SEARCH_USERS)
    for user in data:
        print(user + (', ONLINE' if data[user] else ''))


def quit(s, line=""):
    s.send(json.dumps({'cmd': 'quit'}).encode('utf-8'))
    data = s.recv(1024).decode('utf-8')
    print(data)


def login(s, line):
    try:
        data = {}
        data['cmd'], data['username'], data['password'] = line.split()
        s.send(json.dumps(data).encode('utf-8'))
        response = json.loads(s.recv(1024).decode('utf-8'))
        if response['status'] == 'OK':
            print(info.LOGIN_OK)
            info.USERNAME = data['username']
            info.PASSWORD = data['password']
            return True
        else:
            print(info.LOGIN_ERROR)
    except ValueError:
        print(info.LOGIN_ARG_ERROR)


def add(s, line):
    try:
        data = {}
        data['cmd'], data['friend'] = line.split()
        s.send(json.dumps(data).encode('utf-8'))
        data = json.loads(s.recv(1024).decode('utf-8'))
        if data['status'] == 'OK':
            print(info.ADD_OK)
        else:
            print(info.ADD_ERROR)
    except ValueError:
        print(info.ADD_ARG_ERROR)


def ls(s, line):
    s.send(json.dumps({'cmd': 'ls'}).encode('utf-8'))
    data = json.loads(s.recv(1024).decode('utf-8'))
    print(info.LS_FRIENDS)
    for user in data:
        print(user + (', ONLINE' if data[user] else ''))


def sync(s, line):
    print(info.SYNC)


def recvmsg(s, line=""):
    s.send(json.dumps({'cmd': 'recvmsg'}).encode('utf-8'))
    data = json.loads(s.recv(1024).decode('utf-8'))
    print(info.RECVMSG)
    for friend in data['msg']:
        print(info.MESSAGE % (friend, data['msg'][friend]))


def recvfile(s, line=""):
    s.send(json.dumps({'cmd': 'recvfile'}).encode('utf-8'))
    msg = s.recv(1024).decode('utf-8')
    s.send("ok".encode('utf-8'))
    data = json.loads(msg)
    while data["status"] != "FUCK":
        print(data)
        filename = data["filename"]
        length = int(data["length"])
        with open("/Users/wind/Downloads/" + filename, 'wb') as f:
            max = int((length - 1) / 1024 + 1)
            print("max = " + str(max))
            for i in range(0, max):
                if i < max - 1:
                    content = s.recv(1024)
                else:
                    content = s.recv(length - (max - 1) * 1024)
                s.send("ok".encode('utf-8'))
                f.write(content)
            f.close()
        print("Receive file %s successfully!" % filename)
        msg = s.recv(1024).decode('utf-8')
        s.send("ok".encode('utf-8'))
        data = json.loads(msg)


def chat_fetch_msg(s):
    while True:
        msg = s.recv(1024).decode('utf-8')
        if msg[0: 2] == "OK":
            q.put("OK", block=False)
        else:
            msg = json.loads(msg)
            if 'status' in msg:
                break
            else:
                print(info.MESSAGE % (msg['friend'], msg['msg']))


def chat(s, line):
    try:
        data = {}
        data['cmd'], data['friend'] = line.split()
        s.send(json.dumps(data).encode('utf-8'))
        response = json.loads(s.recv(1024).decode('utf-8'))
        if response['status'] == 'OK':
            print(info.CHAT_OK)
            thread = threading.Thread(target=chat_fetch_msg, args=[s])
            thread.start()
            while True:
                line = input().strip()
                cmd = line.split()[0]
                if cmd == 'exit':
                    exitchat(s)
                    thread.join()
                    break
                else:
                    try:
                        commands['chat'][cmd](s, data['friend'], line)
                    except KeyError:
                        print(info.CMD_NOT_EXIST)
            print(info.CHAT_END)
        else:
            print(data['friend'] + info.CHAT_ERROR)
    except ValueError:
        print(info.CHAT_ARG_ERROR)


def sendmsg(s, friend, line):
    try:
        data = {}
        data['cmd'], data['msg'] = line.split()
        data['friend'] = friend
    except ValueError:
        print(info.SENDMSG_ARG_ERROR)
    else:
        s.send(json.dumps(data).encode('utf-8'))


def sendfile(s, friend, line):
    global q
    try:
        data = {}
        data['cmd'], filename = line.split()
        data['friend'] = friend
        try:
            q = Queue()
            with open("sending/" + filename, 'rb') as f:
                data["filename"] = filename
                s.send(json.dumps(data).encode('utf-8'))
                while q.empty():
                    pass
                q.get(block=False)
                result = f.read(1024)
                i = 0
                while result:
                    print("sending " + str(i))
                    s.send(result)
                    while q.empty():
                        pass
                    q.get(block=False)
                    result = f.read(1024)
                    i = i + 1
                s.send("~".encode("utf-8"))
                while q.empty():
                    pass
                q.get(block=False)
                print(filename + " sent!")
        except Exception:
            print(info.SENDFILE_UPLOAD_ERROR)
    except ValueError:
        print(info.SENDFILE_ARG_ERROR)


def exitchat(s, line=""):
    s.send(b'{"cmd": "exit"}')


def profile(s, line):
    print('username: %s' % info.USERNAME)
    print('password: %s' % info.PASSWORD)
 

commands = {
    'guest':
    {
        'search': search,
        'login': login,
        'quit': quit,
    },
    'user':
    {
        'search': search,
        'add': add,
        'ls': ls,
        'sync': sync,
        'recvmsg': recvmsg,
        'recvfile': recvfile,
        'chat': chat,
        'exit': exitchat,
        'profile': profile,
        'quit': quit,
    },
    'chat':
    {
        'sendmsg': sendmsg,
        'sendfile': sendfile,
    }
}

