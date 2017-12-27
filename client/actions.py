import json
import info
import os
import time
import threading
import dropbox
dbx = dropbox.Dropbox(
    'X5vzMUOqyzAAAAAAAAAAjlHk8HqJM8U07w8B9fDqDp9q03HQ8UhjRlBiGLarrRro'
)


def ping(s, line):
    s.send(json.dumps({'cmd': 'ping'}).encode('utf-8'))
    data = s.recv(1024).decode('utf-8')
    print(data)


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


def recvmsg(s, line=""):
    s.send(json.dumps({'cmd': 'recvmsg'}).encode('utf-8'))
    data = json.loads(s.recv(1024).decode('utf-8'))
    print(info.RECVMSG)
    for item in data['msg']:
        print(info.MESSAGE % (item['friend'], item['msg']))


def recvfile(s, line=""):
    s.send(json.dumps({'cmd': 'recvfile'}).encode('utf-8'))
    data = json.loads(s.recv(1024).decode('utf-8'))
    print(info.RECVFILE)
    for item in data['files']:
        print('Downloading file %s from %s' % (item['path'], item['friend']))
        try:
            dbx.files_download_to_file(
                '/Users/cqb/Downloads/' + os.path.basename(item['path']),
                item['path']
            )
            print('File saved successfully.')
        except Exception:
            print('Downloading failed.')


def chat_fetch_msg(s):
    while True:
        msg = json.loads(s.recv(1024).decode('utf-8'))
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
    try:
        data = {}
        data['cmd'], file_path = line.split()
        data['friend'] = friend
        try:
            with open(file_path, 'rb') as f:
                dbx_path = '/' + info.USERNAME +\
                           '/' + os.path.basename(file_path)
                print(dbx_path)
                dbx.files_upload(f.read(), dbx_path, mode=dropbox.files.WriteMode('overwrite'))
                print(dbx_path)
                data['path'] = dbx_path
        except Exception:
            print(info.SENDFILE_UPLOAD_ERROR)
        else:
            print(info.SENDFILE_OK % data['path'])
            s.send(json.dumps(data).encode('utf-8'))
    except ValueError:
        print(info.SENDFILE_ARG_ERROR)


def exitchat(s, line=""):
    s.send(b'{"cmd": "exitchat"}')


def profile(s, line):
    print('username: %s' % info.USERNAME)
    print('password: %s' % info.PASSWORD)
 

commands = {
    'guest':
    {
        'ping': ping,
        'search': search,
        'login': login,
        'quit': quit,
    },
    'user':
    {
        'ping': ping,
        'search': search,
        'add': add,
        'ls': ls,
        'sync': ls,
        'recvmsg': recvmsg,
        'recvfile': recvfile,
        'chat': chat,
        'quit': quit,
        'profile': profile,
    },
    'chat':
    {
        'sendmsg': sendmsg,
        'sendfile': sendfile,
    }
}

