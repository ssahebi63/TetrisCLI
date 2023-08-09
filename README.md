# TetraShell
TetraShell is an all-in-one Tetris quicksave hacking tool created by Jean Cheng (jgcheng) and Saman Sahebi (saman63). You can use it to hack quicksaves while still making them seem legitimate and climb the leaderboard! Note: This was an academic project from COMP211 System Fundementals 

## Usage
TetraShell will first prompt you to enter the path of a valid quicksave to start hacking. Then, you can use any of the following commands.

### Modify
Implemented like the write up.

### Rank
Implemented like the write up.

### Check
Implemented like the write up.

### Recover
Implemented like the write up.

### Switch
Implemented like the write up.

It checks whether or not the new quicksave is the same as the current one.

### Visualize
Implemented like the write up, except it shows exactly how the save would be displayed in-game instead of a simplified version.

### Help
Implemented like the write up.

### Info
Implemented like the write up.

### Exit
Implemented like the write up.

Additionally, exit can be used at any prompt, including the intial path prompt, (y/n) to switch to a quicksave during pretty recover, and the # of the quicksave to switch to during pretty recover.

## Features

### Short Commands
Implemented like the write up (5 point version).

Additionally, any arguments can also be shortened.

### Improved Prompt
Implemented like the write up.

### Quick-rank
Implemented like the write up.

The default metric is score and the default game number is 5.

### Pretty-recover
Implemented like the write up.

When switching to one of the quicksaves, it checks whether or not the new quicksave is the same as the current one.

It only prints the header if there are any recovered quicksaves, and it reprints the header every 50 quicksaves so you can refer to it again.

It prints the recovered quicksaves in order of recovery.

### Any Case
It accepts arguments in any combination of upper and lower case.

### Colors
The color of the arguments and resulting output oscillate along a gradient between blue and pink for consoles that support color.

## Implementation Decisions

### Argument Valiation
To provide accurate error messages, TetraShell itself handles most of the validation instead of delegating to the programs it calls.

### No Modulo on Color
To change colors, modulo is not used on the variable that keeps track of color. This is to prevent a jump in color when the variable overflows. 
