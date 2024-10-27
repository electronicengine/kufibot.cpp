#!/bin/bash

sox your-audio-file.wav -v 0.1 -t wav - | aplay
