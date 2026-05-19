void itoa(int value, char* buffer)
{
    int i = 0;
    int is_negative = 0;

    if (value == 0)
    {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }

    if (value < 0)
    {
        is_negative = 1;
        value = -value;
    }

    while (value > 0)
    {
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    }

    if (is_negative)
        buffer[i++] = '-';

    buffer[i] = '\0';

    // reverse string
    for (int j = 0, k = i - 1; j < k; j++, k--)
    {
        char tmp = buffer[j];
        buffer[j] = buffer[k];
        buffer[k] = tmp;
    }
}