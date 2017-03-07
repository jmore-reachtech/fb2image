#include <QCoreApplication>
#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QRgb>
#include <QColor>
#include <QDir>

#include <getopt.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FRAMEBUFFER_DEVICE "/dev/fb0"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);    
    const char *filePath = "";
    int width = -1;
    int height = -1;
    int lineLength = -1;

    /* Options needed to run the application */
    while (1) {
        static struct option longOptions[] = {
            { "image-path",  required_argument, 0, 'f' },
            { "help",        no_argument,       0, 'h' },
            { 0,            0, 0,  0  }
        };

        int c = getopt_long(argc, argv, "f:h?", longOptions, 0);

        if (c == -1) {
            break;  // no more options to process
        }

        switch (c) {
        case 'f':
            filePath = optarg;
            break;
        case '?':
        case 'h':
        default:
            printf("Options required\n-f <image file path>\n");
            return 0;
        }

    }

    /* Check required options */
    if (strlen(filePath) == 0) {
        printf("Options required\n-f <image file path>\n");
        return 0;
    }

    /* Check that the image file path is correct */
    QString path(filePath);
    QStringList parts = path.split("/");
    QString folder;

    if (parts.length() > 1){
        folder = parts.at(0);

        for (int i=1; i < parts.length()-1; i++)
            folder.append("/" + parts.at(i));

         QDir dir(folder);
         if (!dir.exists())
         {
             if (dir.mkpath(folder))
                 printf("Created folder %s for screen save.\n", folder.toUtf8().constData());
             else
             {
                 printf("Unable to create folder <%s> for screen save.  Make sure path is correct: %s.\n",
                        folder.toUtf8().constData(), path.toUtf8().constData());
                 return 0;
             }
         }
    }

    /* Open framebuffer to get screen info */
    struct fb_var_screeninfo vscreeninfo;
    struct fb_fix_screeninfo fscreeninfo;
    int fbfd;

    fbfd=open(FRAMEBUFFER_DEVICE, O_RDONLY);
    if(fbfd==-1) {
        printf("Could not open framebuffer");
        return 0;
    }

    /* Fetch variable screen info */
    int ioct=ioctl(fbfd, FBIOGET_VSCREENINFO, &vscreeninfo);
    if(ioct==-1) {
       printf("VSCREEN-IOCTL failed ioct");
       return 0;
    }

    /* Fetch fixed screen info */
    ioct=ioctl(fbfd,FBIOGET_FSCREENINFO,&fscreeninfo);
    if(ioct==-1) {
        printf("FSCREEN-IOCTL failed");
        return 0;
    }

    close(ioct);
    width = vscreeninfo.xres;
    height = vscreeninfo.yres;
    lineLength = fscreeninfo.line_length;
    close(fbfd);

    printf("Screen info found:\nWidth: %d\nHeight: %d\nLine Length: %d\n\n", width, height, lineLength);

    /* Open framebuffer to read pixels */
    QFile file(FRAMEBUFFER_DEVICE);
    if (!file.open(QIODevice::ReadOnly)) {
        printf("Could not open framebuffer device\n");
        return 0;
    }

    QByteArray blob = file.readAll();
    file.close();

    QImage image = QImage((unsigned char*) blob.data(), width, height, lineLength, QImage::Format_RGBX8888);

    QColor color;
    /* We need to go through and fix the red and blue colors */
    for (int x=0; x < image.width(); x++)
    {
        for (int y=0; y < image.height(); y++)
        {
            color = QColor(image.pixel(x,y));
            QColor tempColor = color;

            color.setBlue(tempColor.red());
            color.setRed(tempColor.blue());
            image.setPixel(x, y, color.rgb());
        }
    }


    /* save the image to disk */
    if (image.save(filePath, 0, 100))
        printf("Image saved: %s\n", filePath);
    else
        printf("Image could not be saved.  Check the path or the extension (jpg, png only).: %s\n", filePath);


    return 0;
}
