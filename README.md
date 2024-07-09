# Falling Block Game for TTGO T-Display Board

## Overview

This project implements a falling block game for the TTGO T-Display board using PlatformIO and the ESP-IDF framework. The game challenges players to maneuver a block to avoid falling obstacles, with increasing difficulty over time.

## Features

- **Gameplay**: Players move a block horizontally to avoid falling obstacles using the board's buttons.
- **Scoring**: Score increases when a block misses the player; game ends when the player is hit.
- **Difficulty**: Blocks fall faster and in greater numbers as the game progresses.
- **Score Display**: After game over, the score is displayed for a few seconds.

## Setup

1. **Development Environment**: Set up PlatformIO with ESP-IDF following instructions in Lecture 14.
   
2. **Libraries**: Utilize the TDisplayGraphics library and TTGO Demo code available at [TTGODemo GitHub](https://github.com/a159x36/TTGODemo).

## Instructions

1. **Starting the Game**: Press the left button to start the game.
   
2. **Gameplay**: Use the board's buttons to move the block left and right.
   
3. **Objective**: Avoid being hit by falling blocks; score points for each miss.
   
4. **End Game**: Game ends when the player is hit by a falling block; score is displayed.
   
5. **Score Display**: Score remains visible for a few seconds after game over.

## Sample Scene

![Sample Scene](sample.jpg)
