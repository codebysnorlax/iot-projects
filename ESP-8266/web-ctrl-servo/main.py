import network
import socket
import machine

# Initialize Servo on GPIO2 (D4)
servo = machine.PWM(machine.Pin(2), freq=50)

def set_angle(angle):
    """Convert angle (0-180) to duty cycle (40-115 for SG90)"""
    duty = int((angle / 180) * 75) + 40
    servo.duty(duty)

# Read HTML file
def read_html():
    with open("index.html", "r") as file:
        return file.read()

# Start Web Server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(('', 80))
s.listen(5)

print("Web server running...")

while True:
    conn, addr = s.accept()
    request = conn.recv(1024).decode()
    
    if "GET /servo?angle=" in request:
        try:
            angle = int(request.split("GET /servo?angle=")[1].split(" ")[0])
            set_angle(angle)
            print("Servo angle set to:", angle)
            response = "HTTP/1.1 200 OK\n\nOK"
        except:
            response = "HTTP/1.1 400 Bad Request\n\nInvalid Angle"
    else:
        response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + read_html()

    conn.send(response)
    conn.close()
