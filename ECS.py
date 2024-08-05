import socket
import time

TCP_IP = '10.10.21.108'
TCP_PORT = 9876

pick = (240, 0, -48)
place = {
    "10": (340, 0, -48),
    "20": (340, 100, -48),
    "30": (340, -100, -48)
}
interval = 10

def move_pick(api, pick_x, pick_y, pick_z):
    dType.SetPTPCmd(api, 0, pick_x, pick_y, pick_z, 0, 1)
    dType.SetEndEffectorSuctionCup(api, 1, 1, 1)
    dType.SetWAITCmd(api, 1000, 1)
    return pick_z


def move_place(api, place_x, place_y, place_z):
    dType.SetPTPCmd(api, 0, place_x, place_y, place_z, 0, 1)
    place_z += interval
    dType.SetEndEffectorSuctionCup(api, 0, 0, 1)
    dType.SetWAITCmd(api, 1000, 1)
    return place_z


count = 1
set_flag = 0
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((TCP_IP, TCP_PORT))
while True:
    if(set_flag == 0):
        time.sleep(5)
    set_flag = 1
    msg = "99"
    s_msg = sock.send(msg.encode())
    print("send message : %s" % s_msg)

    r_msg = sock.recv(1024).decode()
    print("received message : %s" % r_msg)

    dType.SetPTPCmd(api, 2, 240, 0, 50, 0, 1)

    if r_msg in place:
        pick_x, pick_y, pick_z = pick
        pick_z = move_pick(api, pick_x, pick_y, pick_z)

        place_x, place_y, place_z = place[r_msg]
        place_z = move_place(api, place_x, place_y, place_z)

        place[r_msg] = (place_x, place_y, place_z)

    if count >= 10:
        time.sleep(5)
        count = 0
        place = {
            "10": (340, 0, -48),
            "20": (340, 100, -48),
            "30": (340, -100, -48)
        }
        dType.SetWAITCmd(api, 2000, 1)
    dType.SetPTPCmd(api, 0, 240, 0, 50, 0, 1)
    time.sleep(7)
    count += 1
