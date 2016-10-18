# KillBox

KillBox is a project build for JacobsHack 2016. 

## Inspiration
Usually if you have physical access to a server, it doesn't matter if you have full disk encryption or not. We wanted to make a server resistant to this. 

## What it does
Our server is connected to a relay, which is controlled by an Arduino. The Arduino has multiple sensors (motion, capacity, tilt and switch). If any of the sensors are triggered the relay will be turned off and the server will lose all power. 
Our plan is to have full disk encryption on the server, so when it gets turned off you wont have access to the data.

### Use-cases
Journalists and whistleblowers could use it to save articles on controversial topics, without having to worry about their freedom. The server could be used to run the Secure Drop project on.

## How we built it
We built it using an Arduino, a Raspberry Pi, a buzzer, multiple sensors and loads of wires. The server is running Raspbian Jessie and Nginx + tor for networking. 

## Challenges we ran into
The capacity sensor was difficult to get right. Another issue was the full disk encryption on the server. We needed to get an old undistributed version of Kali Linux to make this work. 

## Accomplishments that we're proud of
We made the sensors work and the entire system actually works. With the disk encrypted it would be very safe. 

## What we learned
That hardware takes a lot of time and setting up full disk encryption with early-ssh is hard.

## What's next for KillBox
A more production ready implementation. 
