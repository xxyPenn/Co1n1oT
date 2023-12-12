[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/-ZR3sVSa)
# a10-cloud

    * Team Number: 
    * Team Name: 
    * Team Members: 
    * Description of test hardware: (development boards, sensors, actuators, laptop + OS, etc) 

## Github Repo Submission Resources

* [ESE5160 Example Repo Submission](https://github.com/ese5160/example-repository-submission)
* [Markdown Guide: Basic Syntax](https://www.markdownguide.org/basic-syntax/)
* [Adobe free video to gif converter](https://www.adobe.com/express/feature/video/convert/video-to-gif)
* [Curated list of example READMEs](https://github.com/matiassingers/awesome-readme)
* [VS Code](https://code.visualstudio.com/) is heavily recommended to develop code and handle Git commits
  * Code formatting and extension recommendation files come with this repository.
  * Ctrl+Shift+V will render the README.md (maybe not the images though)

## Fill out your assignment answers below

Include all of your relevant source files in the Github repository. You can use Markdown to link GIF images if a video submission is required - or link to a publicly available website hosting the video.

Also check pPDF_A10.pdf in files
## 1. OTAFU
Video See project folder:

https://drive.google.com/file/d/1yAR47tJvU8rI-BEMQbKNFhf3138G1v5b/view?usp=drive_link

## 2. GPIO Function Timing
See project folder, avg time ~2.3ms

Video See project folder:

https://drive.google.com/file/d/1TpeKqyiOTSJPEPrCY00uKZxQXrZIkF69/view?usp=drive_link

## 3. Node-RED Design
-General program flow of your system. What communication does the device need to send to the cloud? What does the cloud do with this information? What does it send back?

![flowdiagram](https://github.com/ese5160/a10-cloud-group-co1n1ot/assets/148792076/e05b3fd8-41f4-42a1-aa8a-0656f5c8ba82)

Mainly the device will send the total amount of the money stored in the piggy bank to the cloud, and the cloud will show the count balance on the UI. The cloud will send back the remote button instruction for lock and unlock operation to the motor.

-List all the topics your system will use. Describe for each one the information that is sent on each topic (is it an integer? A Boolean? An array of integers? A JSON string?) 
Topics:
1.Unlock: User control on the cloud to press the unlock button on UI (send a boolean “true” to Mqtt), and the motor will receive the boolean “true” and implement unlocking/locking behavior.

2.Balance: MCU sends a message containing balance information  (integer variable) to the cloud, and after login, it will display on the UI.

3.Clear Balance: User control on the cloud to press the reset button on UI (send a boolean “true” to Mqtt), and the MCU will clear the balance account, and the display on the piggy bank will clear the balance to zero.

-Describe for each topic in your system: Who generates the data? Who subscribes to each topic?

1.Unlock:  There should be a button on node-red to generate the lock/unlock message, and the motor should subscribe to the topic on MQTT.

2.Balance request: MCU will be the publisher to publish the Balance account variable and the node-red Cloud end will subscribe and show on the UI. Also, the cloud end can publish a clear request to MCU to clear the balance account.

3.Clear Balance: There should be a button on node-red to generate the clear balance message, and the MCU should subscribe to the topic on MQTT, and the lcd on piggy bank will clear the balance when receive message.

-Present how you would divide your MCU Application code into threads, how the threads would communicate, etc.

Using FreeRTOS, there will be I2C to handle the light sensor, SPI to handle the LCD, and there are motor threads to perform the lock/unlock task, and there is a buzzer thread waiting for the I2C light sensor to set the handler to be true, and there is a motor thread waiting for the flag to be set from the cloud.

## 4. Node-RED Implementation
Node-Red:
http://172.190.188.203:1880
Node-Red UI:
http://172.190.188.203:1880/ui

we added node-red-contrib-ui-media module on node-red to display the picture

Video See project folder:
https://drive.google.com/file/d/11ruPOeUMdrW-T6uXNOtXhJf_wYz2ivWn/view?usp=drive_link

Frontend:
![frontend](https://github.com/ese5160/a10-cloud-group-co1n1ot/assets/148792076/11942126-3f36-4445-a3c1-de8da8c3df13)


Backend:
![backend](https://github.com/ese5160/a10-cloud-group-co1n1ot/assets/148792076/f68b7932-7ddf-4bbc-b4a4-5c30f2e860b7)
