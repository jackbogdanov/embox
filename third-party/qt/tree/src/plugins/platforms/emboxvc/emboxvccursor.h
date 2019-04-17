#ifndef QEMBOXVCCURSOR_H
#define QEMBOXVCCURSOR_H

#include <QtGui/QPlatformWindow>
#include <QtGui/QImage>
#include <drivers/video/fb.h>
#include <string.h>
#include <stdio.h>

#include "../fb_base/fb_base.h"
#include <qpainter.h>

QT_BEGIN_NAMESPACE

class QEmboxCursor : public QPlatformSoftwareCursor
{
public:
	QEmboxCursor(QPlatformScreen *s);
    ~QEmboxCursor();

	void emboxCursorRedraw(struct fb_info *fb, int x, int y);
	void emboxCursorReset(struct fb_info *fb);
	void setPainter(QPainter *painter);
	void drawQtCursor();

private:
	void storeDirtyRect(struct fb_info *fb, unsigned char *begin);
	void flushDirtyRect(struct fb_info *fb, unsigned char *begin);
	void drawCursor(struct fb_info *fb, unsigned char *begin);
    void prepareCursor(struct fb_info *fb, unsigned char *begin);
	int imageChanged(struct fb_info *fb, unsigned char *begin);

    int mouseX, mouseY;
    int inited;
    QImage cursor;
    QPainter *cur_painter;
    int cursor_H, cursor_W;
    unsigned char *dirtyRect;
};

QT_END_NAMESPACE

#endif
