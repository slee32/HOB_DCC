# This is boot.py for an esp8266 setup as a server
#
''' @file main.py
    This program contains the code for the boot file for an ESP8266 wifi module that is
    set up as a server. This file is executed on every boot, (supposedly) including 
    wafe-boot from deepsleep.

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

import esp
import network
import gc

# Adafruit's guide for the ESP8266 recommended turning this feature off
esp.osdebug(None)

# Turns on garbage collector which helps clear up unused memory
gc.collect()

###################################################################################
# Functions
###################################################################################

def setup_Station(ssid_name, password_str):
    ''' Sets up the wifi board as a station, or client, and connects to a network. 
    Make sure to import the micropython module "network" in order to ensure proper 
    functionality.
    @param ssid_name the name of the wifi network to connect to.
    @param password_str the password of the wifi network to connect to. '''
    sta_if = network.WLAN(network.STA_IF)
    if not sta_if.isconnected():
        print('connecting to network...')
        sta_if.active(True)
        sta_if.connect(ssid_name, password_str)
        print(sta_if.ifconfig())
        while not sta_if.isconnected():
            pass
    print('network config:', sta_if.ifconfig())

def setup_AP(ssid_name, password_str):
    ''' Sets up the wifi board as an access point, or server, and creates a wifi
    network that other devices can connect to. Make sure to import the micropython
    module "network" in order to ensure proper functionality.
    @param ssid_name the name of the wifi network.
    @param password_str the password of the wifi network. Note that the password 
    needs to be a minimum length or else it will not work. While the exact minimum
    length is not specified, the password '12345678910111213' worked well. '''
    # Make sure to 'import network'
    ap_if = network.WLAN(network.AP_IF)
    ap_if.config(essid=ssid_name, password=password_str)
    ap_if.active(True)
    print('Configured network as AP')
    print('SSID: ', ap_if.config('essid'))
    print('Password: ', password_str)
    print(ap_if.ifconfig())
    print(ap_if.active())

###################################################################################
# Setup code
###################################################################################

# Sets up board as either an access point or station
ssid        = 'HOB Sensors'
password    = '12345678910111213'     # PASSWORD NEEDS TO BE 'LONG'
setup_AP(ssid, password)              # Choose to setup as AP or station