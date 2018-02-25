#!/usr/bin/env python3
import sys
import random

me = 1 # My player id
COLS = 7 # Number of columns
ROWS = 6 # Number of rows
board = [[0 for j in range(COLS)] for i in range(ROWS)]
time_left = 10000 # Track time_left
INPUT_DICT = {'.': 0, '0': 1, '1': 2}

def update_board(text):
    intput_list = text.strip().split(',')
    for i, s in enumerate(intput_list):
        row = i //COLS
        col = i % COLS;
        board[row][col] = INPUT_DICT[s]

def is_move_legal(col):
    """
    A column is only legal if and only if the first row 
    of that column is empty.
    """
    return board[0][col] == 0
    

def make_move(time_left):

    # TODO: Implement bot logic here
    col = random.randint(0, COLS-1)
    
    if is_move_legal(col):
        print('place_disc {}'.format(col))
    else:
        make_move(time_left)


def main():
    for line in sys.stdin:
        command = line.strip().split()
        # Update game field
        if command[0] == "update" and command[2] == "field":
            update_board(command[3])
        elif command[0] == "action" and command[1] == "move":
            make_move(int(command[2]))
        elif command[0] == "settings" and command[1] == "your_botid":
            me = int(command[2]) + 1
        
        
        


if __name__ == '__main__':
    main()
