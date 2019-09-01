<div align="center">

<img src="https://github.com/gnuradio/gr-inspector/blob/master/docs/doxygen/images/logo_body_big.png" />

</div>

# The Inspector (gr-inspector)

## Introduction
This GNU Radio module is developed to realize signal analysis abilities in typical block-structure. The module is capable of the following:
- Energy detection of continuous signals
- Filtering of detected signals
- OFDM parameter estimation (carrier spacing, symbol time)
- Blind OFDM synchronization
- Resampling of signals to constant rate
- 3D Visualisation of FAM data, from gr-specest (not on GR 3.8 yet)
- Using TensorFlow models for AMC (not on GR 3.8 yet)

## Installation

To install the Inspector, the following dependencies are required:

- [GNU Radio](https://github.com/gnuradio/gnuradio) 3.8
    - gr-blocks, gr-filter, gr-fft
    - Optional for tests: gr-analog, gr-channel
- Qt5
- Qwt 6.1.0

This OOT follows the GNU Radio [development
model](https://wiki.gnuradio.org/index.php/GNU_Radio_3.8_OOT_Module_Porting_Guide#Development_Model),
there are different branches for compatibility with different GNU Radio
versions.

Install by the following shell commands:

```
git clone git@github.com:gnuradio/gr-inspector.git
cd gr-inspector
mkdir build
cd build
cmake ..
make -j4
sudo make install
```

## Usage
Get inspired by the examples. There are flowgraphs for each block that should explain how to use them.

For detailed information about each block, visit the Doxygen HTML pages.
## Brief block description

A typical inspector flowgraph could look like this:
![A typical inspector flowgraph](https://grinspector.files.wordpress.com/2016/05/decentral.png)

#### Signal Detector
Performs an energy detection of continuous input signals. A threshold in dB can be set or an autmatic threshold calculation can be used.

#### Inspector GUI
GUI where the spectrum is displayed along with signal markers for the detected signals. Also, signal can be selected manually in this block.

#### Signal Separator
Takes the signal info from the previous blocks and performs a down-mixing, filtering and decimation of each detected signal. The samples for each signal are passed on as a message.

#### Signal Extractor
Adapter block to extract the samples of one signal out of the messages from the Signal Separator. Output is again a complex stream.

Resampling can be used to assure a constant output sample rate of the stream. (Needed for FM demod for instance.)

#### OFDM Estimator
Estimates the parameters subcarrier spacing, symbol time, FFT size and cyclic prefix length for a input signal. The signal should not be oversampled for this block.

#### OFDM Synchronizer
After OFDM parameter estimation, the signal is frequency synced and symbol beginnings are marked with stream tags.

## Google Summer of Code
This GNU Radio module is part of the Google Summer of Code (GSoC) program 2016. The target is to develop a signal analysis / signal intelligence toolbox with the following capabilities:

- Automatic signal detection
- Automatic modulation classification
- OFDM parameter estimation and synchronization
- GUI feedback
- Doxygen documentation

Read the full proposal [here](https://github.com/sbmueller/gsoc-proposal/blob/master/sigint-proposal.pdf).

This project was developed in cooperation with the [Communications Engineering Lab](http://www.cel.kit.edu/) of the Karlsruhe Institute of Technology.

The modulation classification is developed by @chrisruk during ESA Summer of Code in Space program.

## License
This software is Copyright © 2016 Free Software Foundation, Inc. and part of GNU Radio. It is free software, and is released under the [GPLv3](https://www.gnu.org/licenses/gpl-3.0.en.html) License.

## Contact/Help
If you run into problems installing or using gr-inspector, the GNU Radio [mailing list](https://wiki.gnuradio.org/index.php/MailingLists) is the right place to get help. There are many people who will provide assistance. **Please do provide a complete problem description containing**

1. What exactly where you trying to do?
2. What steps did you perform to achieve this?
3. What happend (with complete verbose output)?
4. Why do you think something went wrong? What should have happend instead?

Visit [How to report errors](https://wiki.gnuradio.org/index.php/ReportingErrors) for more detailed instructions.

**Please only use GitHub issues for actual bugs and improvements!** "This command is not working on my machine" is not a bug.

Maintainer of this module:

Sebastian Müller<br/>
gsenpo[at]gmail.com

Christopher Richardson<br/>
chrisrichardsonuk[at]gmail.com

If you want to know more about the development process, please visit the [blog](https://grinspector.wordpress.com/).
