
#include <termios.h>
#include <unistd.h>

void enableRawMode(void)
{
    // function to enter raw mode of the terminal

    struct termios raw;

    // tcgetattr reads the terminal attributes
    tcgetattr(STDIN_FILENO, &raw);

    // dont echo what the user types into the shell by modifying the flags
    raw.c_lflag &= ~(ECHO);

    // now set the terminals attributes to reflect raw mode
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(void)
{
    enableRawMode();

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
        ;
    return 0;
}
