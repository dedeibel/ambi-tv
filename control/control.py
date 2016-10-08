import atexit
import os.path
import errno
from flask import Flask, render_template, url_for, redirect, request
from subprocess import call
app = Flask(__name__)

FIFO_PATH='/var/run/ambi-tv/control_fifo'
PIDFILE='/var/run/ambi-tv/pid'

@app.route('/')
def index():
    make_sure_fifo_opened()
    return render_template('main.html', fifo_exists=fifo_exists(), app_running=app_running(), app_pid=app_pid)

@app.route('/do', methods=['POST'])
def action():
    if 'next-program' in request.form.keys():
        next_program()
    elif 'pause' in request.form.keys():
        pause()
    elif 'halt' in request.form.keys():
        halt()
    return redirect(url_for('index'))

def next_program():
    make_sure_fifo_opened()
    send("NEXT_PROGRAM")

def pause():
    make_sure_fifo_opened()
    send('PAUSE')

def halt():
    make_sure_fifo_opened()
    call(["sudo", "shutdown", "-h", "now"])

def fifo_exists():
    return os.path.exists(FIFO_PATH)

def app_running():
    global app_pid
    app_pid = 'na'
    try:
        app_pid_str = open(PIDFILE).read()
        if len(app_pid_str) != 0:
          app_pid = int(app_pid_str)
        else:
          return False
    except IOError as err:
        print "app pid file not found, so not running"
        return False

    try:
        os.kill(app_pid, 0)
    except OSError as err:
        if err.errno == errno.ESRCH:
            # ESRCH == No such process
            print "pid not found"
            return False
        elif err.errno == errno.EPERM:
            print "no permission to kill, app must be existing"
            # EPERM clearly means there's a process to deny access to
            return True
        else:
            # According to "man 2 kill" possible error values are
            # (EINVAL, EPERM, ESRCH)
            raise
    else:
        print "kill worked, app must be running"
        return True

def send(text):
    control_fifo.write(text + "\n")
    control_fifo.flush()
    print "sent", text

def make_sure_fifo_opened():
    if not control_file_opened:
        if fifo_exists():
            control_fifo.close()
            open_fifo()


def open_fifo():
    global control_file_opened
    global control_fifo
    if fifo_exists():
        print "opening fifo", FIFO_PATH
        control_fifo = open(FIFO_PATH, 'w')
        control_file_opened = True
    else:
        print "fifo does not exist, opening dev null"
        control_fifo = open('/dev/null', 'w')
        control_file_opened = False

def shutdown():
    control_fifo.close()

if __name__ == '__main__':
    #app.debug = True
    atexit.register(shutdown)
    open_fifo()
    app.run(host='0.0.0.0',port=5000)

