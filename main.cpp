



#include "MainWindow.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

extern "C" {
#include "jerror.h"
#include "jpeglib.h"
}

#include <QApplication>
#include <QDebug>
#include <QFile>




#define SIZEOF(object)	((size_t)sizeof(object))


struct PixelRGB {
	uint8_t r, g, b;
};



struct ClientData {
	jmp_buf jb;
	QImage image;
	QFile *infile = nullptr;
	std::vector<uint8_t> srcbuf;
	std::vector<uint8_t> dstbuf;
};

extern "C" size_t jfread_(j_decompress_ptr cinfo, JOCTET *buffer, size_t maxlen)
{
	ClientData *data = (ClientData *)cinfo->client_data;
	return data->infile->read((char *)buffer, maxlen);
}


void my_error_exit(j_common_ptr cinfo)
{
	ClientData *data = (ClientData *)cinfo->client_data;
	longjmp(data->jb, 0);
}

static int min(int a, int b)
{
	return a < b ? a : b;
}


QImage read_jpeg(char const *filename)
{
	QFile file(filename);
	if (!file.open(QFile::ReadOnly)) {
		return QImage();
	}

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	int row_stride;

	ClientData clientdata;
	clientdata.infile = &file;
	cinfo.client_data = &clientdata;

	if (setjmp(clientdata.jb) == 0) {
		cinfo.err = jpeg_std_error(&jerr);
		jerr.error_exit = my_error_exit;
		jpeg_create_decompress(&cinfo);
		jpeg_stdio_src(&cinfo, nullptr);
		jpeg_read_header(&cinfo, TRUE);
		jpeg_start_decompress(&cinfo);

		int scale = 4;
		int width = cinfo.output_width;
		int height = cinfo.output_height;

		{
			int w = width / scale;
			int h = height / scale;
			clientdata.image = QImage(w, h, QImage::Format_RGB888);
			clientdata.image.fill(Qt::black);
		}

		row_stride = width * cinfo.output_components;

		clientdata.srcbuf.resize(row_stride);

		if (scale > 1) {
			clientdata.dstbuf.resize(sizeof(PixelRGB) * width * scale);
		}

		for (int y = 0; y <= height; y++) {
			if (scale > 1 && y > 0 && y % scale == 0) {
				int w = width / scale;
				for (int x = 0; x < w; x++) {
					unsigned int r = 0;
					unsigned int g = 0;
					unsigned int b = 0;
					for (int i = 0; i < scale; i++) {
						uint8_t const *s = clientdata.dstbuf.data() + sizeof(PixelRGB) * (width * i + x * scale);
						for (int j = 0; j < scale; j++) {
							r += s[j * 3 + 0];
							g += s[j * 3 + 1];
							b += s[j * 3 + 2];
						}
					}
					uint8_t *d = clientdata.image.scanLine((y - 1) / scale);
					int n = scale * scale;
					d[x * 3 + 0] = r / n;
					d[x * 3 + 1] = g / n;
					d[x * 3 + 2] = b / n;
				}
			}
			if (y == height) break;

			uint8_t *d;
			if (scale < 2) {
				d = clientdata.image.scanLine(y);
			} else {
				d = clientdata.dstbuf.data() + sizeof(PixelRGB) * width * (y % scale);
			}
			uint8_t *s = &clientdata.srcbuf[0];
			jpeg_read_scanlines(&cinfo, &s, 1);
			if (cinfo.output_components == 1) {
				for (int i = 0; i < width; i++) {
					int y = s[0];
					d[0] = y;
					d[1] = y;
					d[2] = y;
					s++;
					d += 3;
				}
			} else if (cinfo.output_components == 3) {
				for (int i = 0; i < width; i++) {
					d[0] = s[0];
					d[1] = s[1];
					d[2] = s[2];
					s += 3;
					d += 3;
				}
			} else if (cinfo.output_components == 4) {
				for (int i = 0; i < width; i++) {
					int c = s[0];
					int m = s[1];
					int y = s[2];
					int k = s[3];
					d[0] = k * (c) * (228 + m * 27 / 255) / 255 / 255;
					d[1] = k * (160 + c * 95 / 255) * (m) * (241 + y * 14 / 255) / 255 / 255 / 255;
					d[2] = k * (233 + c * 22 / 255) * (127 + m * 128 / 255) * (y) / 255 / 255 / 255;
					s += 4;
					d += 3;
				}
			}
		}
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		return clientdata.image;
	}
	return QImage();
}

int main(int argc, char **argv)
{
	QApplication a(argc, argv);
	MainWindow w;

	QImage img = read_jpeg("C:/a/mimi.jpg");
	w.setImage(img);
	w.show();
	return a.exec();
}
