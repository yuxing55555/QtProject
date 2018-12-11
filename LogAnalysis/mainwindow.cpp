#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	m_drawCurve = false;
	//response the mouse moving,default when the mouse down movind is activing
	this->centralWidget()->setMouseTracking(1);
	this->setMouseTracking(1);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::GetConrolPoint(QVector<QPoint>& oriPoslist,
								QVector<QPointF>& resPoslist)
{
	//generate middle points
	QVector<QPoint> midposList;
	for(int i = 0;i<oriPoslist.size();i++) {
		int nexti = (i+1)%oriPoslist.size();
		int tempx = (oriPoslist[i].x() + oriPoslist[nexti].x())/2.0;
		int tempy = (oriPoslist[i].y() + oriPoslist[nexti].y())/2.0;
		midposList.push_back(QPoint(tempx,tempy));
	}

	QPointF lastControlPos;
	float scale = 0.6;
	for(int i=0;i<midposList.size();i++){
		int backi = 0;
		if(i==0){
			backi = midposList.size()-1;
		}else{
			backi = i-1;
		}
		QPoint temp1 = midposList[i];
		QPoint temp2 = midposList[backi];
		int tempx = (temp1.x() + temp2.x())/2.0;
		int tempy = (temp1.y() + temp2.y())/2.0;
		int offsetx = oriPoslist[i].x() - tempx;
		int offsety = oriPoslist[i].y() - tempy;

		//int addx1 = (temp2.x()+offsetx-oriPoslist[i].x())*scale;
		//int addy1 = (temp2.y()+offsety-oriPoslist[i].y())*scale;

		//int addx2 = (temp1.x()+offsetx-oriPoslist[i].x())*scale;
		//int addy2 = (temp1.y()+offsety-oriPoslist[i].y())*scale;
		if(i==0){
			/*lastControlPos.setX(oriPoslist[i].x()+addx1);
			lastControlPos.setY(oriPoslist[i].y()+addy1);
			resPoslist.push_back(QPointF(oriPoslist[i].x()+addx2,
										 oriPoslist[i].y()+addy2));*/
			lastControlPos.setX(temp2.x()+offsetx);
			lastControlPos.setY(temp2.y()+offsety);
			resPoslist.push_back(QPointF(temp1.x()+offsetx,
										 temp1.y()+offsety));
		}else{
			/*resPoslist.push_back(QPointF(oriPoslist[i].x()+addx1,
										 oriPoslist[i].y()+addy1));
			resPoslist.push_back(QPointF(oriPoslist[i].x()+addx2,
										 oriPoslist[i].y()+addy2));*/
			resPoslist.push_back(QPointF(temp2.x()+offsetx,
										 temp2.y()+offsety));
			resPoslist.push_back(QPointF(temp1.x()+offsetx,
										 temp1.y()+offsety));
		}
	}
	resPoslist.push_back(lastControlPos);
}

void MainWindow::DrawCurve()
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	if(m_clickPosVec.size()==1){
		painter.drawLine(m_clickPosVec.at(0),m_movePos);
	}
	if(m_clickPosVec.size()>1){
		m_clickPosVec.push_back(m_movePos);
		QVector<QPointF> controlPos;
		if(m_drawCurve){
			GetConrolPoint(m_clickPosVec,controlPos);
		}
		for(int i=0;i<m_clickPosVec.size();i++){
			QPoint beginPos = m_clickPosVec.at(i);
			QPoint endPos;
			if(i == m_clickPosVec.size()-1){
				endPos = m_clickPosVec.at(0);
			}else{
				endPos = m_clickPosVec.at(i+1);
			}
			//about draw curve
			QPainterPath path(beginPos);
			if(m_drawCurve){
				path.cubicTo(controlPos.at(i*2), controlPos.at(i*2+1), endPos);
				painter.setPen(QPen(Qt::black, 2));
//				painter.drawPath(path);//darw curve
				painter.fillPath(path, QBrush(Qt::darkYellow));//draw filled curve
			}
			painter.setPen(QPen(Qt::red, 6));
			painter.drawPoint(beginPos);
		}

		/*draw filled plygon*/
		QPainterPath polygonpath;
		painter.setPen(QPen(Qt::blue, 2));
		QPolygon polygon(m_clickPosVec);
		polygonpath.addPolygon(polygon);
		painter.fillPath(polygonpath, QBrush(Qt::cyan));
		m_clickPosVec.removeLast();
		if(!m_drawCurve){
			m_clickPosVec.clear();
		}
	}
}

void MainWindow::paintEvent(QPaintEvent *) {
	if(m_drawCurve)
		DrawCurve();
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
	if(!m_drawCurve)
		return;
	if(e->button() == 0x00000001){
		m_clickPos = e->pos();
	}
}

void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
	if(e->button() == 0x00000001){
		if(e->pos() == m_clickPos){
			m_clickPosVec.append(e->pos());
		}
	}
	if(e->button() == 0x00000002){
		m_drawCurve = false;
		m_clickPosVec.clear();
	}
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
	if(!m_drawCurve)
		return;
	if(m_clickPosVec.size()>0){
		m_movePos = e->pos();
		this->update();
	}
}

void MainWindow::on_actionCurve_triggered()
{
	m_drawCurve = true;
}
