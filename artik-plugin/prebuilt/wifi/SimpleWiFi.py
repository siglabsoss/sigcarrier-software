#!/usr/bin/python3

import subprocess, os, time, curses, tempfile
from subprocess import check_output
from curses import wrapper
from curses.textpad import Textbox, rectangle


class STATIC:
    deviceName = ""
    ethName = ""
    IPaddress = ""
    apList = [0]
    isUbuntu = False
    isSoftAP = False
    NetworkApp = ""
    isNetworkApp = False
    mainWnd = tempfile.TemporaryFile()
    page = 0
    maxpage = 0
    listsize = 0

###########################################################
## curses helper functions
###########################################################

def clearscr(stdscr):
    stdscr.clear()
    STATIC.mainWnd.seek(0)
    try:
        stdscr = curses.getwin(STATIC.mainWnd)
    except:
        stdscr.addstr(3, 3, "curses.getwin() error.")
        stdscr.addstr(4, 3, "check your putwin() routine.")
        stdscr.refresh()
        stdscr.getkey()

    return stdscr
 
###########################################################

def inputwnd(stdscr, msg = None, pos_y = None, pos_x = 2, size_y = 1, size_x = 30):
    tempscr = tempfile.TemporaryFile()
    stdscr.putwin(tempscr)

    if pos_y is None:
        pos_y = curses.LINES - 5
    if msg is not None and (len(msg) + 2 > size_x):
        size_x = len(msg) + 2
    inputWnd = curses.newwin(size_y, size_x, pos_y, pos_x)
    rectangle(stdscr, pos_y - 1, pos_x - 1, pos_y + size_y, pos_x + size_x)
    if msg is not None:
        stdscr.addstr(pos_y - 1, pos_x, " %s " % msg)
    stdscr.refresh()
    box = Textbox(inputWnd)
    box.edit()
    boxtext = box.gather()[:len(box.gather()) - 1]

    tempscr.seek(0)
    stdscr = curses.getwin(tempscr)
    stdscr.refresh()

    return boxtext

###########################################################

def msgbox(msg):
    if type(msg) is int:
        msg = str(msg)
    msglen = len(msg)
    collen = curses.COLS - 10
    strset = []

    for i in range(0, (msglen // collen) + 1):
            if msglen < (i + 1) * collen:
                strset.append(msg[i * collen:msglen])
            else:
                strset.append(msg[i * collen:(i + 1) * collen])

    if msglen > collen:
        msglen = collen

    box = curses.newwin(2 + len(strset), 4 + msglen, 
            (curses.LINES - 1) // 2 - 2, (curses.COLS - 4) // 2 - (msglen // 2))
    box.border(0)

    i = 1
    for j in strset:
        box.addstr(i, 2, j)
        i += 1
    box.refresh()
    box.getkey()
    box.clear()
    box.refresh()


###########################################################
## WiFi main functions
###########################################################

def main(stdscr):
    stdscr.clear()
    stdscr.border(0)
    stdscr.addstr(0, 2, " SimpleWiFi tool")
    p = check_output("cat /etc/os-release", shell=True)
    output = p.decode("utf-8")
    try:
        nidx = output.index("ID=")
        OSname = output[nidx + 3:]
        OSname = OSname[:OSname.index('\n')]
        stdscr.addstr(0, 18, ": %s " % OSname)
        if OSname in "ubuntu":
            STATIC.isUbuntu = True
    except:
        stdscr.addstr(0, 18, ": unknown ")
    stdscr.putwin(STATIC.mainWnd)

    ## Check Soft AP is running
    if os.path.exists("/var/run/hostapd.pid"):
        STATIC.isSoftAP = True
    else:
        STATIC.isSoftAP = False

    ## Check NetworkApp status
    if os.path.exists("/lib/systemd/system/NetworkManager.service"):
        STATIC.NetworkApp = "NetworkManager"
    elif os.path.exists("/lib/systemd/system/connman.service"):
        STATIC.NetworkApp = "connman"

    if len(STATIC.NetworkApp) > 0:
        try:
            p = check_output("systemctl status " + STATIC.NetworkApp, shell=True)
            output = p.decode("utf-8")
        except subprocess.CalledProcessError as e:
            output = e.output.decode("utf-8")
        finally:
            output = output[output.index("Loaded: ") + 8:]
            if output.find(".service; enabled;") > 0:
                if output.find("inactive (dead) since ") > 0:
                    STATIC.isNetworkApp = True

    ## Menu
    userInput = ""
    while userInput is not '0':
        getiwconfig(stdscr)
        stdscr.addstr(3, 3, " 1. Update Device info")
        if STATIC.isSoftAP:
            stdscr.addstr(6, 3, " 4. Go back to Station mode")
        else:
            stdscr.addstr(4, 3, " 2. Connect to Access Point")
            stdscr.addstr(5, 3, " 3. Disconnect")
            stdscr.addstr(6, 3, " 4. Set AP mode")
#        stdscr.addstr(6, 3, " 4. Throughput test")
        stdscr.addstr(8, 3, " 0. Exit")
        stdscr.refresh()

        userInput = inputwnd(stdscr)

        if userInput is '1':
            getiwconfig(stdscr)
        elif userInput is '2':
            connectap(stdscr)
        elif userInput is '3':
            disconnectap()
        elif userInput is '4':
            softap(stdscr)
        elif userInput is '0':
            break
        else:
            msgbox("wrong input")

        stdscr = clearscr(stdscr)

############################################################

def stopnetworkapp():
    if STATIC.NetworkApp is None:
        try:
            p = check_output("systemctl stop wpa_supplicant", shell=True)
            output = p.decode("utf-8")
        except subprocess.CalledProcessError as e:
            output = e.output.decode("utf-8")
    else:
        try:
            p = check_output("systemctl status " + STATIC.NetworkApp, shell=True)
            output = p.decode("utf-8")
        except subprocess.CalledProcessError as e:
            output = e.output.decode("utf-8")
        finally:
            output = output[output.index("Active: ") + 8:]
            if output.startswith("active (running)"):
                p = check_output("systemctl stop " + STATIC.NetworkApp, shell=True)
                STATIC.isNetworkApp = True

############################################################

def softap(stdscr):
    stdscr = clearscr(stdscr)
    stdscr.refresh()

    ## disable Soft AP in a toggle way
    if STATIC.isSoftAP:
        stdscr.addstr(3, 3, "Disabling Soft AP.")
        stdscr.refresh()
        try:
            f = open("/var/run/hostapd.pid", 'r')
            pid = f.read()
            pid = pid[:pid.index('\n')]
            f.close()
            p = check_output("kill " + pid, shell=True)
        
            ## if Broadcom chipset
            if os.path.exists("/sys/module/dhd/"):
                try:
                    p = check_output("dmesg -D", shell=True)
                    p = check_output("modprobe -r dhd", shell=True)
                    p = check_output("modprobe dhd op_mode=0", shell=True)
                    p = check_output("dmesg -E", shell=True)
                except:
                    msgbox("dhd op_mode set fail")
                    return

            stdscr.addstr(3, 21, ".")
            stdscr.refresh()
            p = check_output("systemctl restart wpa_supplicant", shell=True)
            stdscr.addstr(3, 22, ".")
            stdscr.refresh()
        except PermissionError:
            msgbox("Permission denied. Run as root")
        except:
            msgbox("cannot kill previous hostapd")
        else:
            STATIC.isSoftAP = False
            if STATIC.isNetworkApp:
                stdscr.addstr(4, 3, "Restart %s... " % STATIC.NetworkApp)
                stdscr.refresh()
                p = check_output("systemctl start " + STATIC.NetworkApp, shell=True)
                STATIC.isNetworkApp = False
        return

    cfgstr = confighostap(stdscr)
    if cfgstr is None:
        return
    confWnd = curses.newwin(20, curses.COLS - 5, 3, 2)
    confWnd.addstr(0, 0, cfgstr)
    confWnd.refresh()

    ## if Broadcom chipset
    if os.path.exists("/sys/module/dhd/"):
        try:
            p = check_output("dmesg -D", shell=True)
            p = check_output("modprobe -r dhd", shell=True)
            p = check_output("modprobe dhd op_mode=2", shell=True)
            p = check_output("dmesg -E", shell=True)
        except:
            msgbox("dhd op_mode set fail")
            return

    ## stop network application
    stopnetworkapp()

    try:
        p = check_output("ifconfig " + STATIC.ethName + " up", shell=True)
        f = open("/sys/class/net/" + STATIC.ethName + "/carrier", 'r')
        if f.read() is '1':
            try:
                p = check_output(
                        "dhclient " + STATIC.ethName, stderr=subprocess.STDOUT)
            except subprocess.CalledProcessError as e:
                err = e.output.decode("utf-8")
                try:
                    err.index("already running")
                    p = check_output(
                            "dhclient -r", stderr=open(os.devnull, 'w'))
                    p = check_output(
                            "dhclient " + STATIC.ethName, stderr=subprocess.STDOUT)
                except:
                    msgbox("ethernet is connected but dhclient failed to receive IP")
                    f.close()
                    return
        f.close()
    except:
        ## error on ethernet
        pass

    ## configure dnsmasq
    dnsmasq_conf = """# Configured by SimpleWiFi 
no-resolv
no-poll

server=127.0.0.1
server=8.8.8.8
local=/localnet/
"""
    dnsmasq_conf += "interface=" + STATIC.deviceName + "\n"
    dnsmasq_conf += """
bind-interfaces
dhcp-range=192.168.173.100,192.168.173.200,12h

log-queries
log-facility=/var/log/dnsmasq.log
conf-dir=/etc/dnsmasq.d,.rpmnew,.rpmsave,.rpmorig
"""
    try:
        dnsmasq_file = open("/etc/dnsmasq.conf", 'w')
        dnsmasq_file.write(dnsmasq_conf)
        dnsmasq_file.close()

        p = check_output("ifconfig " + STATIC.deviceName + " 192.168.173.1" + " up",
                shell=True)
        if os.path.exists("/var/run/dnsmasq.pid"):
            f = open("/var/run/dnsmasq.pid", 'r')
            pid = f.read()
            pid = pid[:pid.index('\n')]
            f.close()
            os.remove("/var/run/dnsmasq.pid")
            try:
                p = check_output("kill " + pid, shell=True)
            except:
                pass
        p = check_output("dnsmasq -C /etc/dnsmasq.conf", shell=True)
        p = check_output("sysctl net.ipv4.ip_forward=1", shell=True)
        p = check_output("iptables --flush", shell=True)
        p = check_output("iptables -t nat --flush", shell=True)
        p = check_output("iptables --delete-chain", shell=True)
        p = check_output("iptables -t nat --delete-chain", shell=True)
        p = check_output("iptables -t nat -A POSTROUTING -o %s -j MASQUERADE" % STATIC.ethName,
                shell=True)
        p = check_output("iptables -A FORWARD -i %s -j ACCEPT" % STATIC.deviceName,
                shell=True)
    except PermissionError:
        msgbox("Permission denied. Run as root")
        return
    except:
        msgbox("command error")

    try:
        cmd_hostapd = "hostapd -B -P /var/run/hostapd.pid /etc/hostapd/hostapd.conf"
        p = check_output(cmd_hostapd, shell=True)
    except subprocess.CalledProcessError as e:
        err = e.output.decode("utf-8")
        err = err[:err.index('\n')]
        msgbox("CalledProcessError: %s" % err)
        if STATIC.isNetworkApp:
            p = check_output("systemctl start " + STATIC.NetworkApp, shell=True)
            STATIC.isNetworkApp = False
    else:
        STATIC.isSoftAP = True

###########################################################

def confighostap(stdscr):
    ## configure hostapd.conf
    hostapd_conf = "# Configured by SimpleWiFi\n"
    hostapd_conf += "ctrl_interface=/var/run/hostapd\nctrl_interface_group="

    if STATIC.isUbuntu:
        hostapd_conf += "sudo\n"
    else:
        hostapd_conf += "wheel\n"

    hostapd_conf += """
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
driver=nl80211

"""
    hostapd_conf += "interface=" + STATIC.deviceName + "\n"
    ssid = ""
    while len(ssid) is 0:
        curses.echo()
        stdscr.addstr(3, 3, "SSID: ")
        stdscr.refresh()
        ssid = stdscr.getstr().decode("utf-8")
        #ssid = inputwnd(stdscr, "SSID: ", curses.LINES // 2, 5, 1, 20)
        if len(ssid) is 0:
            msgbox("Invalid SSID!")
    hostapd_conf += "ssid=" + ssid + "\n"
    passwd = " "
    while 0 < len(passwd) and len(passwd) < 8:
        stdscr.addstr(4, 3, "Password: ")
        stdscr.refresh()
        passwd = stdscr.getstr().decode("utf-8")
        #passwd = inputwnd(stdscr, "passphrase: ", curses.LINES // 2, 5, 1, 20)
        if 0 < len(passwd) and len(passwd) < 8:
            msgbox("Invalid password! Password must be at least 8 characters")
            stdscr.addstr(4, 1, ' ' * (curses.COLS - 2))
    if len(passwd) is not 0:
        hostapd_conf += """
wpa=3
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
"""
        hostapd_conf += "wpa_passphrase=" + passwd + "\n"
    stdscr.addstr(5, 3, "Operating Channel: ")
    stdscr.refresh()
    op_channel = stdscr.getstr().decode("utf-8")
    #op_channel = inputwnd(stdscr, "Operating Channel: ", curses.LINES // 2, 5, 1, 5)
    if op_channel is '' or op_channel.isdigit() is False:
        msgbox("Invalid channel! Set to default: 6")
        op_channel = '6'
    hostapd_conf += "channel=" + op_channel + "\n"
    if int(op_channel) > 14:
        hostapd_conf += "hw_mode=a\n"
    else:
        hostapd_conf += "hw_mode=g\n"
    curses.noecho()

    stdscr.addstr(6, 3, "< > legacy     < > 11n HT")
    maxpos = 19
    if int(op_channel) > 14:
        stdscr.addstr(6, 33, "< > 11ac VHT")
        maxpos = 34
    str = "Press LEFT/RIGHT to move cursor, ENTER to select, 'b' to back"
    stdscr.addstr(curses.LINES - 1, 5, str)
    stdscr.refresh()

    stdscr.keypad(True)
    curpos = 4
    userInput = ''
    stdscr.move(6, 4)
    stdscr.refresh()

    while userInput != (10 or 13 or curses.KEY_ENTER):
        userInput = stdscr.getch()
        if userInput == curses.KEY_RIGHT:
            curpos += 15
        elif userInput == curses.KEY_LEFT:
            curpos -= 15
        elif userInput is ord('b'):
            return

        if curpos < 4:
            curpos = 4
        elif curpos > maxpos:
            curpos = maxpos

        stdscr.move(6, curpos)
        stdscr.refresh()

    if curpos is 19:
        ## 11n HT selected
        hostapd_conf += "ieee80211n=1"
    elif curpos is 34:
        ## 11ac VHT selected
        hostapd_conf += "ieee80211ac=1"

    stdscr = clearscr(stdscr)
    stdscr.refresh()

    try:
        hostapd_file = open("/etc/hostapd/hostapd.conf", 'w')
        hostapd_file.write(hostapd_conf)
        hostapd_file.close()
    except PermissionError:
        msgbox("Permission denied. Run as root")
        return ""
    else:
        return hostapd_conf

###########################################################

def disconnectap():
    cmd_wpacli_disconnect = "wpa_cli -i" + STATIC.deviceName + " disconnect"
    cmd_ip_flush = "ip addr flush " + STATIC.deviceName
    try:
        p = check_output(cmd_wpacli_disconnect, shell=True, stderr=open(os.devnull, 'w'))
        p = check_output(cmd_ip_flush, shell=True)
    except PermissionError:
        msgbox("Permission denied. Run as root")
    except:
        msgbox("wpa_cli error")
    else:
        msgbox("Disconnected")

###########################################################

def connectap(stdscr):
    stdscr = clearscr(stdscr)

    stdscr = wifiscan(stdscr)
    scancount = 0
    while (STATIC.apList[0] == 0) & (scancount < 5):
        if type(stdscr) is int:
            return
        str = "No AP found yet. Scanning"
        stdscr.addstr(3, 3, str)
        stdscr.addstr(3, 3 + len(str) + scancount, '.')
        stdscr.refresh()
        scancount += 1
        stdscr = wifiscan(stdscr)
        time.sleep(1)

    if STATIC.apList[0] == 0:
        msgbox("No AP to connect!")
        return

    stdscr.keypad(True)
    curpos = 4
    userInput = ''
    stdscr = showaplist(stdscr)
    str = "Press UP/DOWN to move cursor, ENTER to select, 'b' to back"
    stdscr.addstr(curses.LINES - 1, 5, str)
    stdscr.move(4, 5)
    stdscr.refresh()

    while userInput != (10 or 13 or curses.KEY_ENTER):
        userInput = stdscr.getch()
        if userInput == curses.KEY_DOWN:
            curpos += 1
        elif userInput == curses.KEY_UP:
            curpos -= 1
        elif userInput is ord('b'):
            return
        elif (userInput is ord('n')) and (STATIC.page < STATIC.maxpage):
            STATIC.page += 1
            stdscr = showaplist(stdscr)
            str = "Press UP/DOWN to move cursor, ENTER to select, 'b' to back"
            stdscr.addstr(curses.LINES - 1, 5, str)
            stdscr.move(4, 5)
            stdscr.refresh()
        elif (userInput is ord('p')) and (STATIC.page > 0):
            STATIC.page -= 1
            stdscr = showaplist(stdscr)
            str = "Press UP/DOWN to move cursor, ENTER to select, 'b' to back"
            stdscr.addstr(curses.LINES - 1, 5, str)
            stdscr.move(4, 5)
            stdscr.refresh()

        if STATIC.apList[0] > STATIC.listsize:
            if (STATIC.page < STATIC.maxpage) or (STATIC.apList[0] % STATIC.listsize == 0):
                lowerbound = STATIC.listsize
            else:
                lowerbound = STATIC.apList[0] % STATIC.listsize
        else:
            lowerbound = STATIC.apList[0]

        if curpos < 4:
            curpos = 4
        elif curpos > 3 + lowerbound:
            curpos = 3 + lowerbound

        stdscr.move(curpos, 5)
        stdscr.refresh()

    setwpasupplicant(stdscr, STATIC.apList[curpos - 3])

###########################################################

def setwpasupplicant(stdscr, apset):    # apset: ['SSID', 'Frequency', 'RSSI', 'Security']
    stdscr = clearscr(stdscr)
    stdscr.addstr(3, 3, "Connecting to %s" % apset[0])
    stdscr.refresh()

    cmd_wpacli = "wpa_cli -i" + STATIC.deviceName + " "

    p = check_output(cmd_wpacli + "add_network", shell=True)
    network_id = p.decode("utf-8")
    network_id = network_id[:network_id.index('\n')]

    p = check_output(cmd_wpacli + 
            'set_network %s ssid \\"%s\\"' % (network_id, apset[0]), shell=True)
    if not p.decode("utf-8").startswith("OK"):
        msgbox("wpa_cli: set_network failed")
        return

    if apset[3].find("WPA") == -1:
        p = check_output(cmd_wpacli +
                'set_network %s key_mgmt "NONE"' % network_id, shell=True)
        if not p.decode("utf-8").startswith("OK"):
            msgbox("wpa_cli: set_network key_mgmt failed")
            return
    else:
        psk = inputwnd(stdscr, "Input passphrase: ", curses.LINES // 2 - 2, curses.COLS // 2 - 15)
        p = check_output(cmd_wpacli + 
                'set_network %s psk \\"%s\\"' % (network_id, psk), shell=True)
        if not p.decode("utf-8").startswith("OK"):
            msgbox("wpa_cli: set_network psk failed")
            return
    p = check_output(cmd_wpacli + "select_network %s" % network_id, shell=True)
    if not p.decode("utf-8").startswith("OK"):
        msgbox("wpa_cli: select_network failed")
        return

    str = "Waiting to be connected.."
    stdscr.addstr(4, 3, str)
    stdscr.refresh()

    checkcount = 0
    ap_essid = ""
    while (ap_essid != apset[0]) & (checkcount < 10):
        p = check_output(["iwconfig", STATIC.deviceName], stderr=open(os.devnull, 'w'))
        output = p.decode("utf-8")
        try:
            ap_essid = output[output.index("ESSID:") + 7:]
            ap_essid = ap_essid[:ap_essid.index('"')]
        except:
            pass
        checkcount += 1
        stdscr.addstr(4, 3 + len(str), "." * (checkcount % 5))
        if checkcount % 5 is 0:
            stdscr.addstr(4, 3 + len(str), "     ")
        stdscr.refresh()
        time.sleep(1)

    if ap_essid != apset[0]:
        msgbox("Connection failed")
    else:
        str = "Connection established. Receiving IP address.."
        stdscr.addstr(5, 3, str)
        stdscr.refresh()
        try:
            p = check_output(
                    ["dhclient", STATIC.deviceName], stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            err = e.output.decode("utf-8")
            try:
                err.index("already running")
                p = check_output(
                        ["dhclient", "-r"], stderr=open(os.devnull, 'w'))
                p = check_output(
                        ["dhclient", STATIC.deviceName], stderr=subprocess.STDOUT)
            except:
                msgbox("dhclient failed to receive IP")
                return

        checkcount = 0
        while checkcount < 10:
            p = check_output(
                    ["ifconfig", STATIC.deviceName], stderr=open(os.devnull, 'w'))
            output = p.decode("utf-8")
            if output.find("inet ") != -1:
                stdscr.addstr(6, 3, "Done")
                stdscr.refresh()
                return 
            checkcount += 1
            stdscr.addstr(5, 3 + len(str), "." * (checkcount % 5))
            if checkcount % 5 is 0:
                stdscr.addstr(4, 3 + len(str), "     ")
            stdscr.refresh()
            time.sleep(1)
        msgbox("IP is not assigned")

###########################################################

def wifiscan(stdscr):
    stdscr = clearscr(stdscr)

    cmd_wpacli_scan = "wpa_cli -i" + STATIC.deviceName + " scan"
    cmd_wpacli_scanresult = "wpa_cli -i" + STATIC.deviceName + " scan_results"

    try:
        p = check_output(cmd_wpacli_scan, shell=True, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        err = e.output.decode("utf-8")
        err = err[:err.index('\n')]
        msgbox(err)
        return 1
    output = p.decode("utf-8")
    scancount = 0
    while (output.startswith("FAIL-BUSY") & (scancount < 5)):
        str = "Scan failed for device resource busy. Retrying"
        stdscr.addstr(3, 3, str)
        stdscr.addstr(3, 3 + len(str) + scancount, '.')
        stdscr.refresh()
        scancount += 1
        time.sleep(1)
        try:
            p = check_output(cmd_wpacli_scan, shell=True)
        except:
            msgbox("wpa_cli error")
            return 1
        output = p.decode("utf-8")
    if output.startswith("OK"):
        try:
            p = check_output(cmd_wpacli_scanresult, shell=True)
        except:
            msgbox("wpa_cli error")
            return 1
        output = p.decode("utf-8")
        scanlist = output.split('\n')
        STATIC.apList = [0]

        for i in range(1, len(scanlist) - 1):
            ap = scanlist[i].split('\t')
            if int(ap[2]) < -80:
                continue
            apset = []
            apset.append(ap[4])     # SSID
            apset.append(ap[1])     # Frequency
            apset.append(ap[2])     # Signal strength
            ap[3] = ap[3].replace("[ESS]", "")
            apset.append(ap[3])     # Security
            STATIC.apList[0] += 1
            STATIC.apList.append(apset)
    else:
        msgbox("Scan failed")

    return stdscr

###########################################################

def showaplist(stdscr):
    stdscr = clearscr(stdscr)

    str = "       %-14s\t%4s\t%3s\t%s" % ("SSID", "Freq", "RSSI", "Security")
    stdscr.addstr(2, 3, str)
    j = 1

    totalAP = STATIC.apList[0]
    STATIC.listsize = curses.LINES - 7
    STATIC.maxpage = totalAP // STATIC.listsize
    if totalAP % STATIC.listsize == 0:
        STATIC.maxpage -= 1

    if totalAP > STATIC.listsize:
        if (STATIC.page < STATIC.maxpage) or (totalAP % STATIC.listsize == 0):
            for i in range(1, STATIC.listsize + 1):
                str = (" < > %-16s\t%4s\t%3s\t%s"
                        % (STATIC.apList[STATIC.page * STATIC.listsize + i][0],
                            STATIC.apList[STATIC.page * STATIC.listsize + i][1],
                            STATIC.apList[STATIC.page * STATIC.listsize + i][2],
                            STATIC.apList[STATIC.page * STATIC.listsize + i][3]))
                stdscr.addstr(3 + j, 3, str)
                j += 1
        else:
            for i in range(1, totalAP % STATIC.listsize + 1):
                str = (" < > %-16s\t%4s\t%3s\t%s"
                        % (STATIC.apList[STATIC.page * STATIC.listsize + i][0],
                            STATIC.apList[STATIC.page * STATIC.listsize + i][1],
                            STATIC.apList[STATIC.page * STATIC.listsize + i][2],
                            STATIC.apList[STATIC.page * STATIC.listsize + i][3]))
                stdscr.addstr(3 + j, 3, str)
                j += 1
        stdscr.addstr(curses.LINES - 2, 7,
                "Total %d AP found. %d/%d page"
                % (totalAP, STATIC.page, STATIC.maxpage))
        stdscr.addstr(3 + j, 5,
                "* There is more access points. "
                "Press 'n' for next, 'p' for previous")

    else:
        for i in range(1, totalAP + 1):
            str = (" < > %-16s\t%4s\t%3s\t%s"
                    % (STATIC.apList[i][0], STATIC.apList[i][1], STATIC.apList[i][2], STATIC.apList[i][3]))
            stdscr.addstr(3 + j, 3, str)
            j += 1

    return stdscr

###########################################################

def getiwconfig(stdscr):
    rectangle(stdscr, 0, curses.COLS - 33, 6, curses.COLS - 1)
    stdscr.refresh()

    infoWnd = curses.newwin(5, 31, 1, curses.COLS - 32)
    p = check_output("iwconfig", stderr=subprocess.STDOUT)
    output = p.decode("utf-8")
    
    ## get WiFi device name
    try:
        devstr = output[output.index("wl"):]
    except ValueError:
        infoWnd.addstr(0, 0, "Cannot find any WiFi devices")
        infoWnd.refresh()
        return

    devstr = devstr[:devstr.index(' ')]
    STATIC.deviceName = devstr
    infoStr = "{:>13}".format("Device Name: ") + STATIC.deviceName + '\n'
    
    ## get ethernet device name
    try:
        devstr = output[output.index("eth"):]
    except ValueError:
        try:
            devstr = output[output.index("enp"):]
        except ValueError:
            devstr = " "
    finally:
        devstr = devstr[:devstr.index(' ')]
        STATIC.ethName = devstr

    ## retrieve AP connection information
    if STATIC.isSoftAP:
        f = open("/etc/hostapd/hostapd.conf", 'r')
        str = f.read()
        SoftAPchannel = str[str.index("channel=") + 8:]
        SoftAPchannel = SoftAPchannel[:SoftAPchannel.index('\n')]
        infoStr += "{:>13}".format("AP channel: ") + SoftAPchannel + '\n'

        idx = str.find("ieee80211")
        if idx < 0:
            SoftAPband = str[str.index("hw_mode=") + 8:]
            SoftAPband = SoftAPband[:SoftAPband.index('\n')]
        else:
            SoftAPband = str[idx + 9:]
            SoftAPband = SoftAPband[:SoftAPband.index('=')]
        infoStr += "{:>13}".format("AP band: ") + "802.11" + SoftAPband + '\n'
        f.close()
    else:
        try:
            ap_essid = output[output.index("ESSID:") + 7:]
            ap_essid = ap_essid[:ap_essid.index('"')]
            ap_mac = output[output.index("Access Point:") + 14:]
            ap_mac = ap_mac[:ap_mac.index(' ')]
            if ap_mac == "Not-Associated":
                infoStr += "{:>13}".format("AP MAC: ") + "Not connected\n"
            else:
                infoStr += "{:>13}".format("AP MAC: ") + ap_mac + '\n'
                infoStr += "{:>13}".format("AP SSID: ") + ap_essid + '\n'
        except:
            pass
    infoStr += getifconfig(STATIC.deviceName, stdscr)

    infoWnd.addstr(0, 0, infoStr)
    infoWnd.refresh()

###########################################################

def getifconfig(devstr, stdscr):
    retstr = ""

    #if devstr == "":
    #    return "Cannot find any WiFi devices"
 
    p = check_output(["ifconfig"])
    output = p.decode("utf-8")
    ## Bring up WiFi device
    try:
        str = output[output.index(devstr):]
    except:
        stdscr.addstr(curses.LINES - 1, 1, devstr + " device is not up. Bring up the device")
        stdscr.refresh()
        try:
            cmd_ifconfig_up = "ifconfig " + devstr + " up"
            p = check_output(cmd_ifconfig_up, shell=True)
            p = check_output(["ifconfig"])
            output = p.decode("utf-8")
            str = output[output.index(devstr):]
        except:
            return "Failed to bring up a device: " + devstr

    ## Retrieve device MAC address
    try:
        HWaddrIdx = str.index("HWaddr ") + 7
    except:
        try:
            HWaddrIdx = str.index("ether ") + 6
        except:
            return "MAC address fail!"
    HWaddr = str[HWaddrIdx:HWaddrIdx + 17]
    retstr = "{:>13}".format("My MAC: ") + HWaddr + '\n'

    ## Retrieve IP address
    try:
        IPaddrIdx = str.index("inet ") + 5
    except:
        retstr += "{:>13}".format("IP address: ") + "not assigned"
        return retstr

    IPaddr = str[IPaddrIdx:IPaddrIdx + 22]
    if IPaddr.startswith("addr:"):
        IPaddr = IPaddr[5:]
    IPaddr = IPaddr[:IPaddr.index("  ")]
    retstr += "{:>13}".format("IP address: ") + IPaddr
    STATIC.IPaddress = IPaddr

    return retstr

###########################################################

wrapper(main)
