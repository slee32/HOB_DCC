# This is main.py for an esp8266 setup as a server
#
''' @file main.py
    This program contains the code for the main file for an ESP8266 wifi module that is
    set up as a server. This file creates a simple HTML webpage that displays the
    status of the GPIO pins (configured as input pins) of the board and updates them
    about every 1 second. Note that the webpage can be accessed by connecting to the 
    network created by the board, opening a web browser, and going to "192.168.4.1" 
    (this address shouldn't change, but it can be found 'getaddrinfi()'')

    @b Revisions:
    11-28-2018 SRW Original File
    12-01-2018 SRW Modified code to include server setup

    @b Usage:
    This file is intended to be run on an ESP8266 wifi module that has been setup to
    run micropython. 

    @b License:
    This file is copyright 2018 by SRW and released under the Lesser GNU 
    Public License, version 3. It intended for educational use only, but its use
    is not limited thereto, except by the fact that it's not really useful for
    anything else, so nobody in his right (or left) mind would try to use it. 
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
    TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.'''

''' Run this code if the name of this file is "main.py". '''
if __name__ == "__main__":

    import machine
    import socket

    ''' Sets up pins of the wifi module to be digital input pins. This will display
    the status of the hall-effect sensors. '''
    pins = [machine.Pin(i, machine.Pin.IN) for i in (0, 2, 4, 5, 12, 13, 14, 16)]

    ''' Creates a simple webpage to display the status of the input pins. '''
    html = """<!DOCTYPE html>
    <html>
        <head> <title>ESP8266 Pins</title> 
        <meta http-equiv="refresh" content="1"/> 


        </head>
        <body> <h1>ESP8266 Pins</h1>
            <table border="1"> <tr><th>Pin</th><th>Value</th></tr> %s </table>
        </body>
    </html>
    """

    ''' Creates a socket object to establish a connection with other devices. '''
    addr = socket.getaddrinfo('0.0.0.0', 80)[0][-1]
    s = socket.socket()
    s.bind(addr)            # "binds" socket object to an address
    s.listen(1)
    print('listening on', addr)         # "listens" for devices to connect

    ''' This loop allows the server to run until it loses power or is reset. '''
    while True:
        cl, addr = s.accept()   # accepts a client that is trying to connect
        print('client connected from', addr)

        cl_file = cl.makefile('rwb', 0)
        while True:
            line = cl_file.readline()
            if not line or line == b'\r\n':
                break
        rows = ['<tr><td>%s</td><td>%d</td></tr>' % (str(p), p.value()) for p in pins]
        response = html % '\n'.join(rows)
        cl.send(response)
        cl.close()