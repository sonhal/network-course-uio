# Congestion control

### A simple router model

- Buffers to handle same time input that should go on the same output

### Goal of congestion control is to keep the buffer/queues small

### [Problem] Reduced throughput from increased capacity

### Global congestion collapse

Packet loss is a sign of congestion


### TCP sliding window

### TCP congestion controll: Tahoe

- Congestion control
- Flow control

### AIMD

- fairness

Has some problems (simulation shown in slides)

#### How large should a queue be?
Bandwidth * delay product (BDP)

### Using delay as a signal

- Delay grows before loss, so can react earlier, lit delay


### More problems with delay as a signal


### Active Queue Management

### Explicit Congestion Notification (ECN)
Instead off dropping, set a bit for the receiver to notify the sender off.

Not deployed much today

### Delay-based congestion control
