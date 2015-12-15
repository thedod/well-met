**Heads up:** at the moment, the randomness of the output of this device is [not satisfactory](http://pastebin.com/wAbWhTfP), but at least we're testing it now :wink:. Stay tuned...

---------


This is a variation on @robseward's [arduino-rng](https://github.com/robseward/arduino-rng/). It uses the same [circuit](http://robseward.com/misc/RNG2/#circuit) [with an additional button &mdash; see below], but 
requires a board that supports [keyboard emulation](https://www.arduino.cc/en/Reference/MouseKeyboard) [I used a [Micro](https://www.arduino.cc/en/Main/ArduinoBoardMicro)].

Instead of generating an endless stream of random bits, each time you reset the device, it:

* Generates a small fixed length buffer [currently, 36 bytes]
* Encodes it as base64 [48 bytes in our case], Note: You'll need to include @adamvr's [arduino-base64](https://github.com/adamvr/arduino-base64) library for this
* Makes the result url-safe by converting `/` to `_` [and &mdash; while we're at it &mdash; `+` to `-`, because it might get interpreted as a space in some circumstances]
* A button [connected to `A1` in this example] "types" the result, emulating a keyboard. You can send *the same string* repeatedly [to generate a new one &mdash; reset the device]

In addition to the 30ms long LED blinks during the calibration phase, there's a 10ms long flash after the generation of each byte, and once the buffer is ready
the LED becomes steadily lit, to indicate that the buffer is ready and you can press the button in order to "type" the string (pressing the button before that has no effect).
The entire process may take up to 3 minutes.

![This is what it looks like](https://lut.im/uldZKOlbeA/q7St5DK5Aw1TVc50.jpg)

### Why?

This can be used as a *shared secret* between two or more peers who are able to meet face to face (hence the name `well-met`).

Although modern cryptography (since the invention of the [Diffie-Hellman](https://en.wikipedia.org/wiki/Diffie%E2%80%93Hellman_key_exchange) key exchange)
enables secure communication with remote peers without the need to secretly exchange keys, this creates a vulnerability to [man in the middle](https://en.wikipedia.org/wiki/Man-in-the-middle_attack)
attacks, where Eve can impersonate as Alice to Bob. Solutions to this problem (e.g. the [SMP](https://en.wikipedia.org/wiki/Socialist_millionaire) protocol used in order to authenticate
peers in [OTR](https://en.wikipedia.org/wiki/Off-the-Record_Messaging) messaging), rely [if I understand correctly] on *some* shared secret known to both parties.
It doesn't have to be strong enough to serve as the actual encrypyion key, but its strength may still be an issue (if I understand correctly,
this is why [pond](https://pond.imperialviolet.org/user.html) offers its "card trick").

Although it would be daft to *rely* on the ability of peers to meet face to face, this is somethimes the case (e.g. people meet at conventions and cryptoparties),
and it's not trivial to generate strong enough shared secrets (especially if you're meeting many people at the same time).

Even less secure mediums (e.g. [etherpads](https://pad.riseup.net) or IRC servers, where peers are forced to trust a singe-point-of-failure server) could benefit from a device like
*well met*, and hopefully &mdash; people will [come up](https://github.com/thedod/well-met/wiki) with other uses for such gadgets.

### *[New]:* kb_debug

As suggested by @yuvadm, there's now a `kb_debug` mode, where instead of generating a single 48 byte base64 string [to be sent whenever the button is pressed],
it continuously generates a string, sends it (followed by the new-line character), and repeats the process. This way we can open a text editor and let the device
"type" the data into it, decode the base64 with [`b64decode.py`], and test the data's randomness (e.g. with [rngtest](http://linuxcommand.org/man_pages/rngtest1.html)).

To activate `kb_debug`, keep the button pressed until calibration ends.

### Disclaimer

I'm not a cryptographer. I'm just a dude who cares about his privacy. Comments are welcome.

-------------------------------------------------

Original README by @robseward

-------------------------------------------------

Arduino True Random Number Generator
----

Instructions for constructing the circuit can be found here: http://robseward.com/misc/RNG2/

### Eagle Files

These are **UNTESTED**. (I designed the PCB but never got around to building it. If you make one, please let me know of any mistakes &mdash; or better yet make a pull request!)
