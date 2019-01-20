# Washing-Machine
Communication, sensors and actuators for my old washing machine with NodeMCU and Arduino code

<p align="center">
<img src="https://github.com/celatzur/Washing-Machine/blob/master/images/Protoboard_01x.jpeg" width="320" height="277" />
</p>
<p align="center">
<img src="https://github.com/celatzur/Washing-Machine/blob/master/images/WashingMachine_HandSketchxC.jpeg" width="400" height="130" />
</p>

To detect the end of my old washing machine cycle, and then send an email and beeps. Activate the washing machine with a servomotor through internet. (Create WebServer, Sense Light, Send eMail, Activate Servomotor).

<p align="center">
<img src="https://github.com/celatzur/Washing-Machine/blob/master/images/Screenshot_01.png" width="290" height="177" />
</p>

TTD: Check the LDR once every 5 minutes, and sleep after sending the mail to save battery, or between readings. Send mail if battery is low
