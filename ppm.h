#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>

/**
 *  Code basically copied from OpenglExamples (texture loading)
 * 
 * */

unsigned char* ppmRead(std::string filename, int* width, int* height){
    FILE* fp;
    int i, w, h, d;
    unsigned char* image;
    char head[70];			/* max line <= 70 in PPM (per spec). */

    fp = fopen(filename.data(), "rb");
    if (!fp) {
        perror(filename.data());
        return NULL;
    }

    /* grab first two chars of the file and make sure that it has the
       correct magic cookie for a raw PPM file. */
    fgets(head, 70, fp);
    if (strncmp(head, "P6", 2)) {
    	fprintf(stderr, "[FATAL] %s: Not a raw PPM file\n", filename.data());
    	return NULL;
    }

    /* grab the three elements in the header (width, height, maxval). */
    i = 0;
    while(i < 3) {
        fgets(head, 70, fp);
        if (head[0] == '#')		/* skip comments. */
            continue;
        if (i == 0)
            i += sscanf(head, "%d %d %d", &w, &h, &d);
        else if (i == 1)
            i += sscanf(head, "%d %d", &h, &d);
        else if (i == 2)
            i += sscanf(head, "%d", &d);
    }

    /* grab all the image data in one fell swoop. */
    image = (unsigned char*)malloc(sizeof(unsigned char)*w*h*3);
    fread(image, sizeof(unsigned char), w*h*3, fp);
    fclose(fp);

    *width = w;
    *height = h;
    return image;
}

bool LoadPPM(int bind, std::string filename) {
    GLint w, h, i;
    GLubyte* texture;

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, bind);
    texture = (GLubyte*) ppmRead( filename, &w, &h );
    if (texture == NULL)
        return false;
    
    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, w, h, GL_RGB, GL_UNSIGNED_BYTE, texture );
    free( texture );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ); 
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glDisable( GL_TEXTURE_2D );
    return true;
}