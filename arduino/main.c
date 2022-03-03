#include <stdio.h>
#include <math.h>

#define RADIUS 48

// format values[angle][radius][x or y]

int main()
{
    FILE *fp;

    fp = fopen("./array.txt", "w+");

    fprintf(fp, "{\n");
    for (int theta = 0; theta <= 90; theta++)
    {
        fprintf(fp, "\t{\n");
        double rad = theta * (M_PI / 180.0);

        for (int r = 1; r <= RADIUS; r++)
        {
            fprintf(fp, "\t\t{ ");
            for (int var = 0; var < 2; var++)
            {
                if (var == 0)
                {
                    double x = r * cos(rad);
                    int x_r = round(x);
                    fprintf(fp, "%d, ", x_r);
                }
                else
                {
                    double y = r * sin(rad);
                    int y_r = round(y);
                    fprintf(fp, "%d ", y_r);
                }
            }
            if (r == RADIUS)
            {
                fprintf(fp, "}\n");
            }
            else
            {

                fprintf(fp, "},\n");
            }
        }
        if (theta == 90)
        {

            fprintf(fp, "\t}\n");
        }
        else
        {
            fprintf(fp, "\t},\n");
        }
    }
    fprintf(fp, "};\n");
}