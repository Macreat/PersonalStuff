#include <stdio.h>

// This program demonstrates a simple number guessing logic in C.
//  It defines a function that compares a guessed number against a fixed target (555)
//  program provides feedback based on whether the guess is too low, too high, or correct.

void guessNumber(int guess)
{
    // TODO: write your code here
    if (guess == 555)
    {
        printf("correct guess\n");
    }
    else if (guess < 555)
    {
        printf("your guess is too low\n");
    }
    else
    {
        printf("your guess is too high\n");
    }
}

int main()
{
    guessNumber(500);
    guessNumber(600);
    guessNumber(555);
}