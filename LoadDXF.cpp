#include "StdAfx.h"
#include "MainFrm.h"
#include "FMDoc.h"
#include "LoadDXF.h"

//****************************************************************************************************
extern void DPtoVP(double x,double y,int*X,int*Y);
extern void VPtoDP(int x,int y,double*X,double*Y);
extern int DLtoVL(double l);
extern double VLtoDL(int l);


//为了提高重画速度，所以加的全局函数
double xMinScreen,yMinScreen,xMaxScreen,yMaxScreen;
//为了提高重画速度，所以加的全局函数，这是为了计算屏幕的边界与元素对象的边界是不是相交
BOOL IsRectCross(double minx,double miny,double maxx,double maxy);

//检查矩形是否与屏幕矩形相交
BOOL IsRectCross(double minx,double miny,double maxx,double maxy)
{
	if(minx>xMaxScreen||maxx<xMinScreen||miny>yMaxScreen||maxy<yMinScreen) //两个矩形区域不相交
		return FALSE; //如不相交函数返回0
	else 
		return TRUE; //如果相交就返回1
}



//默认构造函数
CLoadDXF::CLoadDXF()
{
	j = 0;
	Fill = -1;
	Numble = 0;
}
//默认析构函数
CLoadDXF::~CLoadDXF()
{

}


//读取图层的数据
void CLoadDXF::addLayer(const DL_LayerData& data)
{
	CMainFrame* pFrame=(CMainFrame*)(AfxGetApp()->m_pMainWnd);
	CFMDoc *pDoc=(CFMDoc *)pFrame->GetActiveDocument();
	CString Layer ;
	COLORREF Color;
	BOOL Delete;
	int nTemp;
	//得到图层名字
	Layer = data.name.c_str();
	//得到颜色代码
	nTemp = attributes.getColor();  
	if ( nTemp < 0)
	{
		Delete = TRUE;   //如果小于0就说明这个图层是隐藏的
	} 
	else
	{
		Delete = FALSE;  //如果大于或等于0就说明这个图层是要显示的
	}
	//将颜色代码转换为RGB
	Color = LoadColorreF(nTemp);    
	//如果Delete为FALSE，将得到的图层对像装入Array
	if (!Delete)  
	{
	  pDoc->AddLayer(Layer,Color,Delete); 
	}
	//如果用下面是话，就会将CAD里所有的数据读取，这样……将国将不国
	//pDoc->AddLayer(Layer,Color,Delete); 
}

//读取直线的数据
void CLoadDXF::addLine(const DL_LineData& data)
{
	CMainFrame* pFrame=(CMainFrame*)(AfxGetApp()->m_pMainWnd);
	CFMDoc *pDoc=(CFMDoc *)pFrame->GetActiveDocument();
	CString Layer ;
	COLORREF Color;
	BOOL Delete;

	//得到直线的起末点数据
	double X1 = data.x1;
	double Y1 = data.y1;
	double X2 = data.x2;
	double Y2 = data.y2;

	//得到图层名字
	Layer = attributes.getLayer().c_str();
	//得到图层的颜色
	Color = GetLayerColor(Layer);
	//得到这个直线对像是否要隐藏
	Delete = GetLayerDelete( Layer);
	//如果Delete为FALSE,这个直线对像就要显示
	if (!Delete)  
	{
		pDoc->AddLine(Layer,Color,Delete,X1,Y1,X2,Y2);
	}
	//如果用下面是话，就会将CAD里所有的数据读取，这样……
	//pDoc->AddLine(Layer,Color,Delete,X1,Y1,X2,Y2);

}
//读取弧线的数据
void CLoadDXF::addArc(const DL_ArcData& data)
{
	CMainFrame* pFrame=(CMainFrame*)(AfxGetApp()->m_pMainWnd);
	CFMDoc *pDoc=(CFMDoc *)pFrame->GetActiveDocument();
	CString Layer ;
	COLORREF Color;
	BOOL Delete;

	double CirceX = data.cx;
	double CirceY = data.cy;
	double CirceR = data.radius;
	//开源库里给的是角度，这里要将角度转换为弧度，角度转换为弧度公式：弧度=角度×(π ÷180 )
	double Angle1 = data.angle1 * (pi/180);
	double Angle2 = data.angle2 * (pi/180);

	Layer = attributes.getLayer().c_str();
	Color = GetLayerColor(Layer);
	Delete = GetLayerDelete( Layer);
	if (!Delete)
	{
		pDoc->AddArc(Layer,Color,Delete,CirceX,CirceY,CirceR,Angle1,Angle2);
	}

}
//读取圆的数据
void CLoadDXF::addCircle(const DL_CircleData& data)
{
	CMainFrame* pFrame=(CMainFrame*)(AfxGetApp()->m_pMainWnd);
	CFMDoc *pDoc=(CFMDoc *)pFrame->GetActiveDocument();
	CString Layer ;
	COLORREF Color;
	BOOL Delete;

	double CirceX = data.cx;
	double CirceY = data.cy;
	double CirceR = data.radius;

	Layer = attributes.getLayer().c_str();
	Color = GetLayerColor(Layer);
	Delete = GetLayerDelete( Layer);
	if (!Delete)
	{
		pDoc->AddPCircle(Layer,Color,Delete,CirceX,CirceY,CirceR);
	}

}
//读取多段线的数据//////闭合标志位，1表示闭合，0表示非闭合
void CLoadDXF::addPolyline(const DL_PolylineData& data)
{
	//多线段标志，1=关闭，0=开
	Fill =(int)data.flags; 
	//顶点数
	Numble = (int)data.number;   
	if (Numble>0)
	{
		m_PointList = new PointStruct[Numble+1];
	}
}
//顶点对像，描绘多线实体// bulge = 凸点 默认值是0  ,,(data.x, data.y, data.z,data.bulge);bulge = 凸点 默认值是0
void CLoadDXF::addVertex(const DL_VertexData& data)
{
	if ( Numble>0 )
	{
		m_PointList[j].x = data.x;
		m_PointList[j].y = data.y;
		j++;
	}
	if ( j == Numble )
	{
		CMainFrame* pFrame=(CMainFrame*)(AfxGetApp()->m_pMainWnd);
		CFMDoc *pDoc=(CFMDoc *)pFrame->GetActiveDocument();
		CString Layer;
		COLORREF Color;
		BOOL Delete ;
		Layer = attributes.getLayer().c_str();
		Color = GetLayerColor(Layer);
		Delete = GetLayerDelete( Layer);
		if (!Delete)
		{
			pDoc->AddPLine(Layer,Color,Delete,Numble,m_PointList,Fill);
		}
		j = 0;
		BOOL Fill = -1;
		int Numble = 0;
		delete m_PointList;
	}
}

//*************************************************************************************************************

//图形的旋转
void CDraw::RotatePoint (double baseX , double baseY , double angle , double *xx, double *yy)
{
	double xx1,yy1;
	xx1 = *xx;
	yy1 = *yy;
	//得到点相对于基点的弧度
	double angle1 = GetAngle(baseX ,baseY ,xx1,yy1);
	//得到点与基点距离
	double j1 = CalDisp(baseX ,baseY ,xx1,yy1);
	//得到旋转后点的横坐标
	xx1 = baseX + j1*cos(angle + angle1);
	//得到旋转后点的纵坐标
	yy1 = baseY + j1*sin (angle + angle1);
	//返回旋转后的横坐标
	*xx = xx1;
	//返回旋转后的纵坐标
	*yy = yy1;
}
//图形的旋转
double CDraw::GetAngle(double baseX , double baseY , double xx , double yy)
{
	double r ,ang;
	if ( fabs (baseX - xx) <=0.001 && fabs( baseY - yy) <=0.001) //如果基点和计算的点是同一个点
	{
		return 0;
	}
	if (fabs(baseX - xx) <=0.001) //如果两个点在一个竖直线上
	{
		if (yy>baseY)
			ang = (double)(pi/2);
		else
			ang = (double)(pi*3/2);
		return ang;
	}
	else
	{
		r = (double) sqrt ((xx - baseX) * (xx - baseX) + (yy - baseY) * (yy - baseY));
		double kk = (xx - baseX) /r;
		ang = (double) acos (kk);
		if (yy< baseY)
			ang = (double)(pi*2) - ang;
		return ang;
	}
}

//计算缩放的函数
void CDraw::ScalePoint (double x_Base , double y_Base , double x_Scale , double y_Scale , double* xx , double* yy)
{
	*xx = x_Base + (*xx - x_Base) * x_Scale;
	*yy = y_Base + (*yy - y_Base) * y_Scale;
}

//图形元素基类串行化
void CDraw::Serialize(CArchive& ar)
{
	if (ar.IsStoring())  //如果是存储操作
	{
		ar<<m_Layer<<m_Color<<b_Delete;
	}
	else                //如果是读取操作
	{
		ar>>m_Layer>>m_Color>>b_Delete;
	}
}

//图层串行化
IMPLEMENT_SERIAL(CLayer,CObject,1)
	void CLayer::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar<<m_Layer<<m_Color<<b_Delete;
	} 
	else
	{
		ar>>m_Layer>>m_Color>>b_Delete;
	}

}

//直线的绘制
void CLine::Draw(CDC *pDC,int m_DrawMode,int m_DrawMode1,COLORREF BackColor)
{
	int x1,y1,x2,y2;
	if(b_Delete)	     //如果已经处于删除状态,退出函数
		return;

	//检查此对象是否与屏幕矩形相交
	double minx,miny,maxx,maxy;
	GetRect(&minx,&miny,&maxx,&maxy);
	if(!IsRectCross(minx,miny,maxx,maxy))
		return;

	int LineType = 0;    //线形
	int LineWide = 1;    //线宽
	COLORREF ColorPen = m_Color;   //线色

	if(m_DrawMode1==1)	//被鼠标选择选择后特殊显示，如果图形要特殊显示
	{
		
		int r = GetRValue(m_Color);
		int g = GetGValue(m_Color);
		int b = GetBValue(m_Color);
		ColorPen = RGB(255-r,255-g,255-b);
		
		if(LineType!=2) //如果图形元素不是用的第2种线形（虚线），则采用第2种线形
			LineType=2; 
		else
			LineType++;  //如果图形元素用的第2种线形（虚线），则采用第3种线形
	}
	else if(m_DrawMode1==2)	 //如果要指定颜色绘制，将笔色设为屏幕底色BackColor
		ColorPen = BackColor;

	CPen pen((int)LineType,LineWide,ColorPen); //创建并初始化一个CPen对像，设定画笔的线型.宽度.颜色
	CPen* pOldPen = pDC->SelectObject(&pen);   //在绘图对像中选入CPen对像

	if(m_DrawMode==0)             //如果采用第0种绘制模式
		pDC->SetROP2(R2_COPYPEN); //设定覆盖的绘制模式
	else if(m_DrawMode==1)        //如果采用第1种绘制模式
		pDC->SetROP2(R2_NOTXORPEN);     //则设置反写的绘制模式

	DPtoVP(m_X1,m_Y1,&x1,&y1);    
	DPtoVP(m_X2,m_Y2,&x2,&y2);
	pDC->MoveTo(x1,y1);
	pDC->LineTo(x2,y2);
	pDC->SelectObject(pOldPen);//恢复画笔
	pDC->SetROP2(R2_COPYPEN);//这是为了框选之后框子消失
}

//得到直线对像的最大边缘矩形,函数Line::GetRect作用：计算并返回一条直线的矩形边界
void CLine::GetRect(double *minX,double *minY,double *maxX,double *maxY)
{
	*minX = min(m_X1,m_X2);   //计算并返回最小x值
	*maxX = max(m_X1,m_X2);   //计算并返回最大x值
	*minY = min(m_Y1,m_Y2);   //计算并返回最小y值
	*maxY = max(m_Y1,m_Y2);   //计算并返回最大y值
}

//函数：CLine::IsPoint
//作用：判断点是否在直线上，参数（x,y)是点中点的坐标，jl为点中的距离范围
//返回：1-点中直线 0-没有点中
BOOL CLine::IsPoint(double x,double y,double jl , double blc)
{
	double xx,x1,x2,y1,y2;

	//如果图形元素做了删除标志，返回没有选中标志
	if(b_Delete)
		return FALSE;

	//得到每条直线的边界矩形
	GetRect(&x1,&y1,&x2,&y2);

	 //如果点不在边界矩形内,函数返回FALSE
	if(!(x>=x1&&x<=x2&&y>=y1&&y<=y2))
		return FALSE;

	//得到按中点到直线距离
	xx=PointLine(x,y,m_X1,m_Y1,m_X2,m_Y2);
	if(xx<jl)//如果选中
		return TRUE;
	return FALSE;
}
//计算下线的旋转
void CLine::Rotate(double baseX , double baseY , double angle)
{
	RotatePoint( baseX , baseY , angle , &this->m_X1 , &this->m_Y1 );
	RotatePoint( baseX , baseY , angle , &this->m_X2 , &this->m_Y2 );
}

//计算直线的缩放
void CLine::Scale (double x_Base , double y_Base , double x_Scale , double y_Scale )
{
	ScalePoint(x_Base ,y_Base ,x_Scale ,y_Scale ,&m_X1,&m_Y1);
	ScalePoint(x_Base ,y_Base ,x_Scale ,y_Scale ,&m_X2,&m_Y2);
}

//直线的移动
void CLine::Move(double x_Move , double y_Move)
{
	m_X1 += x_Move;
	m_X2 += x_Move;
	m_Y1 += y_Move;
	m_Y2 += y_Move;
}

//框选图形元素判断的虚函数，判断直线是不是与矩形相交或在其内部
BOOL CLine:: IsRectangle(PointStruct *pPoint ,double blc) 
{
	//假如直线与矩形相交，返回TRUE
	if (LineRan(m_X1 , m_Y1 , m_X2 , m_Y2, pPoint, blc))
	{
		return TRUE;
	}
	//假如直线只要有一点在矩形内部，返回TRUE
	if (PointRgnRec(m_X1 ,m_Y1, pPoint,blc))
	{
		return TRUE;
	}
	//否则直线未与矩形相交，也不在矩形内部，就返回FALSE
	return FALSE;
}

//工具栏组合框选择图层
BOOL CLine::IsLayerCombox(CString Lyaer)
{
	if (m_Layer == Lyaer)
	{
		return TRUE;
	}
	return FALSE;
}

//元素顶点的蓝色方框
void CLine::BlueBox(CDC* pDC)
{
	int x1,y1,x2,y2;
	DPtoVP(m_X1,m_Y1,&x1,&y1);    
	DPtoVP(m_X2,m_Y2,&x2,&y2);
	CPen pen , *pOldPen;
	pen.CreatePen(0,1,RGB(128,128,128));
	pOldPen=pDC->SelectObject(&pen);
	CBrush brush ,*pOldBrush;
	brush.CreateSolidBrush(RGB(0,255,0));
	pOldBrush = pDC->SelectObject(&brush);
	pDC->Rectangle(x1-4,y1-4,x1+4,y1+4);
	pDC->Rectangle(x2-4,y2-4,x2+4,y2+4);
	pDC->Rectangle((x1+x2)/2-4,(y1+y2)/2-4,(x1+x2)/2+4,(y1+y2)/2+4);
	pDC->SelectObject(pOldPen);     //恢复画笔
	pen.DeleteObject();
	pDC->SelectObject(pOldBrush); //恢复刷子
	brush.DeleteObject();
}

IMPLEMENT_SERIAL(CLine,CObject,1)
	void CLine::Serialize(CArchive& ar)
{
	CDraw::Serialize(ar);
	if (ar.IsStoring())
	{
		ar<<m_X1<<m_Y1<<m_X2<<m_Y2;
	} 
	else
	{
		ar>>m_X1>>m_Y1>>m_X2>>m_Y2;
	}
}

//****************************************************************************************************

//多段线绘制
void CPline::Draw(CDC *pDC,int m_DrawMode,int m_DrawMode1,COLORREF BackColor)
{
	int x1,y1;
	if(b_Delete)	//如果已经处于删除状态,退出函数
		return;

	//检查此对象是否与屏幕矩形相交
	double minx,miny,maxx,maxy;
	GetRect(&minx,&miny,&maxx,&maxy);
	if(!IsRectCross(minx,miny,maxx,maxy))
		return;

	int LineType = 0;    //线形
	int LineWide = 1;    //线宽
	COLORREF ColorPen = m_Color;   //线色

	if(m_DrawMode1==1)	//被鼠标选择选择后特殊显示，如果图形要特殊显示
	{
		int r = GetRValue(m_Color);
		int g = GetGValue(m_Color);
		int b = GetBValue(m_Color);
		ColorPen = RGB(255-r,255-g,255-b);

		if(LineType!=2) //如果图形元素不是用的第2种线形（虚线），则采用第2种线形
			LineType=2; 
		else
			LineType++;  //如果图形元素用的第2种线形（虚线），则采用第3种线形
	}
	else if(m_DrawMode1==2)	 //如果要指定颜色绘制，将笔色设为屏幕底色BackColor
		ColorPen = BackColor;

	CPen pen((int)LineType,LineWide,ColorPen); //创建并初始化一个CPen对像，设定画笔的线型.宽度.颜色
	CPen* pOldPen = pDC->SelectObject(&pen);   //在绘图对像中选入CPen对像

	//CBrush *pBrush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH));
	//CBrush *pOldBrush = pDC->SelectObject(pBrush);
	pDC->SelectStockObject(NULL_BRUSH);   //设置不填充状态，透明画刷

	if(m_DrawMode==0)             //如果采用第0种绘制模式
		pDC->SetROP2(R2_COPYPEN); //设定覆盖的绘制模式
	else if(m_DrawMode==1)        //如果采用第1种绘制模式
		pDC->SetROP2(R2_NOTXORPEN);     //则设置反写的绘制模式

	POINT* ppoint;
	ppoint = new POINT[m_Numble+1];
	for (int i = 0 ; i < m_Numble ; i++)  //将实际坐标转换成屏幕点阵坐标，存入ppoint
	{
		DPtoVP(m_PointList[i].x , m_PointList[i].y , &x1 , &y1);
		ppoint[i].x = x1;
		ppoint[i].y = y1;
	}

	if(! b_Fill)   //绘制连续直线
	{
		ppoint[m_Numble] = ppoint[0];
		pDC->MoveTo(ppoint[0].x , ppoint[0].y);
		for(int i = 0; i< m_Numble ;i++)
			pDC->LineTo(ppoint[i].x , ppoint[i].y);
	}

	else
	{
		pDC->Polygon(ppoint,m_Numble);  //绘制多边形 
	}

	delete ppoint;
	pDC->SelectObject(pOldPen);     //恢复画笔
	//pDC->SelectObject(pOldBrush); //恢复刷子
	pDC->SetROP2(R2_COPYPEN);//这是为了框选之后框子消失
}

//多段线对像的最大边缘矩形//函数:CPline::GetRect作用:计算并返回一条连续直线的边界矩形
void CPline::GetRect(double *minX,double *minY,double *maxX,double *maxY)
{
	double x1,y1,x2,y2;
	//用第一个顶点坐标初始化变量
	x1 = m_PointList[0].x;
	y1 = m_PointList[0].y;
	x2 = m_PointList[0].x;
	y2 = m_PointList[0].y;
	//从第二个顶点开始循环
	for (int i = 1; i<m_Numble; i++)
	{
		x1 = min(x1, m_PointList[i].x);    //得到最小x坐标
		x2 = max(x2, m_PointList[i].x);    //得到最大x坐标
		y1 = min(y1, m_PointList[i].y);    //得到最小y坐标
		y2 = max(y2, m_PointList[i].y);    //得到最大y坐标
	}
	*minX = x1; 
	*maxX = x2; 
	*minY = y1; 
	*maxY = y2;
}

//函数：PLine::IsPoint
//功能：判断点是否在连续直线上，参数（x,y)是点中点的坐标，jl为点中的距离范围，
//返回：1-点中连续直线或封闭区域 0-没有点中
BOOL CPline::IsPoint(double x,double y,double jl,double blc)
{
	int i;
	double xx,x1,y1,x2,y2;

	//如果图形元素做了删除标志，函数返回FALSE
	if(b_Delete)
		return FALSE;

	//得到连续直线或多边形区域的边界矩形
	GetRect(&x1,&y1,&x2,&y2); 

	//如果按中点不在边界矩形内,则不能选中
	if(!(x>=x1-jl&&x<=x2+jl&&y>=y1-jl&&y<=y2+jl))
		return FALSE;	

	//如果是连续直线
	if(!b_Fill)			
	{
		for(i=0;i<m_Numble-1;i++)  //依次对连续直线的每条直线进行判断
		{
			xx=PointLine(x,y,m_PointList[i].x,m_PointList[i].y,m_PointList[i+1].x,m_PointList[i+1].y);
			if(xx<jl)                     //如果选中
				return TRUE;	
		}
	}
	//如果是多边形区域
	else				
	{
		 //如果点在封闭区域内
		if(PointRgn(x,y,m_Numble,m_PointList,blc))   
			return TRUE;
	}
	return FALSE; //没有选中连续直线或封闭区域，返回0
}

//计算多段线的旋转
void CPline::Rotate(double baseX , double baseY , double angle)
{
	for (int i = 0 ; i < this->m_Numble ; i++)
	{
		RotatePoint(baseX,baseY,angle,&m_PointList[i].x , &m_PointList[i].y);
	}
}
//计算多段线的缩放
void CPline::Scale (double x_Base , double y_Base , double x_Scale , double y_Scale )
{
    for ( int i = 0 ; i< m_Numble ; i++)
    {
		ScalePoint(x_Base ,y_Base ,x_Scale ,y_Scale ,&m_PointList[i].x,&m_PointList[i].y);
    }

}

//多段线的移动
void CPline::Move(double x_Move , double y_Move)
{
	for (int i = 0; i<m_Numble ;i++)
	{
		m_PointList[i].x += x_Move;
		m_PointList[i].y += y_Move;
	}
}

//框选图形元素判断的虚函数，判断直线是不是与矩形相交或在其内部
BOOL CPline:: IsRectangle(PointStruct *pPoint ,double blc) 
{
	//假如多段线中的一个直线与矩形相交，返回TRUE
	if (PLineRan(m_Numble,m_PointList, pPoint, blc))
	{
		return TRUE;
	}
	//假如多段线中的第一点与矩形相交，返回TRUE
	if (PointRgnRec(m_PointList[0].x , m_PointList[0].y , pPoint , blc))
	{
		return TRUE;
	}
	return FALSE;
}


//工具栏组合框选择图层
BOOL CPline::IsLayerCombox(CString Lyaer)
{
	if (m_Layer == Lyaer)
	{
		return TRUE;
	}
	return FALSE;
}

//元素顶点的蓝色方框
void CPline::BlueBox(CDC* pDC)
{
	CPen pen , *pOldPen;
	pen.CreatePen(0,1,RGB(0,128,0));
	pOldPen=pDC->SelectObject(&pen);
	CBrush brush ,*pOldBrush;
	brush.CreateSolidBrush(RGB(0,0,255));
	pOldBrush = pDC->SelectObject(&brush);
	int x1,y1;
	for (int i = 0 ; i < m_Numble ; i++) 
	{
		DPtoVP(m_PointList[i].x , m_PointList[i].y , &x1 , &y1);
		pDC->Rectangle(x1-4,y1-4,x1+4,y1+4);
	}
	pDC->SelectObject(pOldPen);     //恢复画笔
	pen.DeleteObject();
	pDC->SelectObject(pOldBrush); //恢复刷子
	brush.DeleteObject();
}

IMPLEMENT_SERIAL(CPline,CObject,1)
	void CPline::Serialize(CArchive& ar)
{
	int i;
	CDraw::Serialize(ar);
	if (ar.IsStoring())
	{
		ar<<b_Fill<<m_Numble;
		for (i = 0 ; i < m_Numble ; i++)
		{
			ar<<m_PointList[i].x<<m_PointList[i].y;
		}
	} 
	else
	{
		ar>>b_Fill>>m_Numble;
		m_PointList = new PointStruct[m_Numble];
		for (i = 0;i<m_Numble;i++)
		{
			ar>>m_PointList[i].x>>m_PointList[i].y;
		}
	}

}

//***********************************************************************************************************************

//绘制圆
void CCircle::Draw(CDC *pDC,int m_DrawMode,int m_DrawMode1,COLORREF BackColor)
{
	if(b_Delete)	//如果已经处于删除状态,退出函数
		return;
	int x ,y ,r;

	//检查此对象是否与屏幕矩形相交
	double minx,miny,maxx,maxy;
	GetRect(&minx,&miny,&maxx,&maxy);
	if(!IsRectCross(minx,miny,maxx,maxy))
		return;

	int LineType = 0;    //线形
	int LineWide = 1;    //线宽
	COLORREF ColorPen = m_Color;   //线色

	if(m_DrawMode1==1)	//被鼠标选择选择后特殊显示，如果图形要特殊显示
	{
		int r = GetRValue(m_Color);
		int g = GetGValue(m_Color);
		int b = GetBValue(m_Color);
		ColorPen = RGB(255-r,255-g,255-b);

		if(LineType!=2) //如果图形元素不是用的第2种线形（虚线），则采用第2种线形
			LineType=2; 
		else
			LineType++;  //如果图形元素用的第2种线形（虚线），则采用第3种线形
	}
	else if(m_DrawMode1==2)	 //如果要指定颜色绘制，将笔色设为屏幕底色BackColor
		ColorPen = BackColor;

	CPen pen((int)LineType,LineWide,ColorPen); //创建并初始化一个CPen对像，设定画笔的线型.宽度.颜色
	CPen* pOldPen = pDC->SelectObject(&pen);   //在绘图对像中选入CPen对像

	//CBrush *pBrush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH));
	//CBrush *pOldBrush = pDC->SelectObject(pBrush);
	pDC->SelectStockObject(NULL_BRUSH);

	if(m_DrawMode==0)             //如果采用第0种绘制模式
		pDC->SetROP2(R2_COPYPEN); //设定覆盖的绘制模式
	else if(m_DrawMode==1)        //如果采用第1种绘制模式
		pDC->SetROP2(R2_NOTXORPEN);     //则设置反写的绘制模式

	DPtoVP(m_CirceX,m_CirceY,&x,&y);  //计算得到圆心的逻辑坐标
	r = DLtoVL(m_CirceR);             //计算得到半径的逻辑坐标长度
	pDC->Ellipse(x-r,y-r,x+r,y+r);    //绘制圆
	pDC->SelectObject(pOldPen);       //恢复画笔
	pDC->SetROP2(R2_COPYPEN);//这是为了框选之后框子消失

}

//函数：CCircle::GetRect作用：计算并返回一条个圆的矩形边界
void CCircle::GetRect(double *minX,double *minY,double *maxX,double *maxY)
{
	*minX = m_CirceX - m_CirceR;  //得到并返回最小x值
	*minY = m_CirceY - m_CirceR;  //得到并返回最大x值
	*maxX = m_CirceX + m_CirceR;  //得到并返回最小y值
	*maxY = m_CirceY + m_CirceR;  //得到并返回最大y值
}

//函数：CCircle::IsPoint
//功能：判断点是否在圆上，参数（x,y)是点中点的坐标，jl为点中的距离范围，
//返回：1-点中圆或圆形区域，0-没有选中
BOOL CCircle::IsPoint(double x,double y,double jl, double blc)
{
	double xx;
	//如果图形元素做了删除标志，函数返回FALSE
	if(b_Delete)
		return FALSE;

	//点到圆心的距离
	xx=CalDisp(x,y,m_CirceX,m_CirceY);	

	//如果按点到圆的距离小于jl
	if((xx >(m_CirceR-jl)) && (xx <(m_CirceR+jl)))
		return TRUE;

	//如果点在圆内
	if(xx<=m_CirceR) 
		return TRUE;
// 	
	//如果没有点中圆或者填充圆，返回0
	return FALSE;    
}

//计算圆的旋转
void CCircle::Rotate(double baseX , double baseY , double angle)
{
	RotatePoint(baseX,baseY,angle, &this->m_CirceX, &this->m_CirceY);
}

//计算圆的缩放
void CCircle::Scale (double x_Base , double y_Base , double x_Scale , double y_Scale )
{
	m_CirceX = x_Base + (m_CirceX - x_Base) * x_Scale;
	m_CirceY = y_Base + (m_CirceY - y_Base) * y_Scale;
	m_CirceR = m_CirceR * x_Scale;
}

//圆的移动
void CCircle::Move(double x_Move , double y_Move)
{
	m_CirceX += x_Move;
	m_CirceY += y_Move;
}

BOOL CCircle:: IsRectangle(PointStruct *pPoint ,double blc) 
{
	//假如圆心在矩形中，返回TRUE
	if (PointRgnRec(m_CirceX, m_CirceY , pPoint , blc))
	{
		return TRUE;
	}
	return FALSE;

}

//工具栏组合框选择图层
BOOL CCircle::IsLayerCombox(CString Lyaer)
{
	if (m_Layer == Lyaer)
	{
		return TRUE;
	}
	return FALSE;
}

//元素顶点的蓝色方框
void CCircle::BlueBox(CDC* pDC)
{
	CPen pen , *pOldPen;
	pen.CreatePen(0,1,RGB(128,0,0));
	pOldPen=pDC->SelectObject(&pen);
	CBrush brush ,*pOldBrush;
	brush.CreateSolidBrush(RGB(255,0,0));
	pOldBrush = pDC->SelectObject(&brush);

	int x,y,r;
	DPtoVP(m_CirceX,m_CirceY,&x,&y);  //计算得到圆心的逻辑坐标
	r = DLtoVL(m_CirceR);             //计算得到半径的逻辑坐标长度

	pDC->Rectangle(x-4,y-4,x+4,y+4);
	pDC->Rectangle(x-r-4,y-4,x-r+4,y+4);
	pDC->Rectangle(x+r-4,y-4,x+r+4,y+4);
	pDC->Rectangle(x-4,y-r-4,x+4,y-r+4);
	pDC->Rectangle(x-4,y+r-4,x+4,y+r+4);
	
	pDC->SelectObject(pOldPen);     //恢复画笔
	pen.DeleteObject();
	pDC->SelectObject(pOldBrush); //恢复刷子
	brush.DeleteObject();
}

IMPLEMENT_SERIAL(CCircle,CObject,1)
	void CCircle::Serialize(CArchive& ar)
{
	CDraw::Serialize(ar);
	if (ar.IsStoring())
	{
		ar<<m_CirceX<<m_CirceY<<m_CirceR;
	} 
	else
	{
		ar>>m_CirceX>>m_CirceY>>m_CirceR;
	}
}

//*****************************************************************************************************

//绘制弧线
void CArc::Draw(CDC *pDC,int m_DrawMode,int m_DrawMode1,COLORREF BackColor)
{
	if(b_Delete)	//如果已经处于删除状态,退出函数
		return;

	//检查此对象是否与屏幕矩形相交
	double minx,miny,maxx,maxy;
	GetRect(&minx,&miny,&maxx,&maxy);
	if(!IsRectCross(minx,miny,maxx,maxy))
		return;

	int x ,y ,rr;
	int x1,x2,y1,y2;

	int LineType = 0;    //线形
	int LineWide = 1;    //线宽
	COLORREF ColorPen = m_Color;   //线色

	if(m_DrawMode1==1)	//被鼠标选择选择后特殊显示，如果图形要特殊显示
	{
		int r = GetRValue(m_Color);
		int g = GetGValue(m_Color);
		int b = GetBValue(m_Color);
		ColorPen = RGB(255-r,255-g,255-b);

		if(LineType!=2) //如果图形元素不是用的第2种线形（虚线），则采用第2种线形
			LineType=2; 
		else
			LineType++;  //如果图形元素用的第2种线形（虚线），则采用第3种线形
	}
	else if(m_DrawMode1==2)	 //如果要指定颜色绘制，将笔色设为屏幕底色BackColor
		ColorPen = BackColor;

	CPen pen((int)LineType,LineWide,ColorPen); //创建并初始化一个CPen对像，设定画笔的线型.宽度.颜色
	CPen* pOldPen = pDC->SelectObject(&pen);   //在绘图对像中选入CPen对像

	if(m_DrawMode==0)             //如果采用第0种绘制模式
		pDC->SetROP2(R2_COPYPEN); //设定覆盖的绘制模式
	else if(m_DrawMode==1)        //如果采用第1种绘制模式
		pDC->SetROP2(R2_NOTXORPEN);     //则设置反写的绘制模式

	DPtoVP(m_CirceX,m_CirceY,&x,&y);  //将圆心实际坐标转为屏幕坐标
	rr = DLtoVL(m_CirceR);            //计算得到半径的逻辑坐标长度          
	//得到起点的坐标
	x1 = x + DLtoVL(m_CirceR * cos(m_Angle1));
	y1 = y - DLtoVL(m_CirceR * sin(m_Angle1));
	//得到终点的坐标
	x2 = x + DLtoVL(m_CirceR * cos(m_Angle2));
	y2 = y - DLtoVL(m_CirceR * sin(m_Angle2));

	pDC->Arc(x-rr,y-rr,x+rr,y+rr,x1,y1,x2,y2);  //绘制圆弧
	pDC->SelectObject(pOldPen);                 //恢复画笔
	pDC->SetROP2(R2_COPYPEN);//这是为了框选之后框子消失
}

//函数：Aarc::GetRect作用：计算一条弧的边界矩形，弧的坐标在arc1内
void CArc::GetRect(double *minX,double *minY,double *maxX,double *maxY)
{
	double x,y,x1,x2,y1,y2;
	int i ;
	//  得到圆弧起点坐标
	x = m_CirceX + (double)(cos(m_Angle1) * m_CirceR);
	y = m_CirceY + (double)(sin(m_Angle1) * m_CirceR);
	//给边界矩形初值：
	x1 = x;
	y1 = y;
	x2 = x;
	y2 = y;
	// 得到圆弧终点坐标
	x = m_CirceX + (double)(cos(m_Angle2) * m_CirceR);
	y = m_CirceY + (double)(sin(m_Angle2) * m_CirceR);
	//得到最小和最大坐标 
	x1 = min(x,x1);
	x2 = max(x,x2);
	y1 = min(y,y1);
	y2 = max(y,y2);

	for (i = 0 ; i < 4; i++)
	{
		if (IsInArc(pi/2*i))
		{
			if (i == 0)
			{
				x = m_CirceX + m_CirceR;
				y = m_CirceY;
			}
			if (i == 1)
			{
				x = m_CirceX ;
				y = m_CirceY + m_CirceR ;
			}
			if (i == 2)
			{
				x = m_CirceX - m_CirceR;
				y = m_CirceY;
			}
			if (i == 3)
			{
				x = m_CirceX ;
				y = m_CirceY - m_CirceR ;
			}

			x1 = min(x,x1);
			x2 = max(x,x2);
			y1 = min(y,y1);
			y2 = max(y,y2);
		}
	}
	*minX = x1; 
	*maxX = x2; 
	*minY = y1; 
	*maxY = y2;
}
BOOL CArc::IsInArc(double angle)
{
	if ((m_Angle1 < m_Angle2 && angle < m_Angle2 && angle > m_Angle1) 
		|| m_Angle1 > m_Angle2 && (!(angle <= m_Angle1 && angle >= m_Angle2)))
		return TRUE;
	else
		return FALSE;
}

//函数：CArc::IsPoint
//功能：判断点是否在圆弧上，参数（x,y)是点中点的坐标，jl为点中的距离范围，
//返回：1-点中圆弧，index返回点中的图形元素的序列号，index1返回唯一识别号
BOOL CArc::IsPoint(double x,double y,double jl, double blc)
{
	//调用CCircle类的IsPoint判断是否选中了圆
	if(CCircle::IsPoint(x,y,jl,blc))
	{
		//按中点到圆心的距离
		double xx=CalDisp(x,y,m_CirceX,m_CirceY);
		//按中点相对于圆心的弧度
		double ang=(double)acos((x-m_CirceX)/xx);

		if(y-m_CirceY<0)
			ang=(double)pi*2-ang;

		//如果按中的点相对于圆心的弧度在圆弧的弧度范围内，则返回TRUE
		if(IsInArc(ang))
			return TRUE;
	}
	return FALSE;//如果没有点中圆弧，返回0
}

//计算圆弧的旋转
void CArc::Rotate(double baseX , double baseY , double angle)
{
	//pProgram->
	RotatePoint(baseX,baseY,angle, &this->m_CirceX, &this->m_CirceY);
	this->m_Angle1+=angle;
	this->m_Angle2+=angle;
}

//计算圆弧的缩放
void  CArc::Scale (double x_Base , double y_Base , double x_Scale , double y_Scale )
{
	m_CirceX = x_Base + (m_CirceX - x_Base) * x_Scale;
	m_CirceY = y_Base + (m_CirceY - y_Base) * y_Scale;
	m_CirceR = m_CirceR * x_Scale;
}

//工具栏组合框选择图层
BOOL CArc::IsLayerCombox(CString Lyaer)
{
	if (m_Layer == Lyaer)
	{
		return TRUE;
	}
	return FALSE;
}

//元素顶点的蓝色方框
void CArc::BlueBox(CDC* pDC)
{
	CPen pen , *pOldPen;
	pen.CreatePen(0,1,RGB(128,128,0));
	pOldPen=pDC->SelectObject(&pen);
	CBrush brush ,*pOldBrush;
	brush.CreateSolidBrush(RGB(255,255,0));
	pOldBrush = pDC->SelectObject(&brush);

	int x,y,r;
	DPtoVP(m_CirceX,m_CirceY,&x,&y);  //计算得到圆心的逻辑坐标
	r = DLtoVL(m_CirceR);             //计算得到半径的逻辑坐标长度

	pDC->Rectangle(x-4,y-4,x+4,y+4);
	pDC->Rectangle(x-r-4,y-4,x-r+4,y+4);
	pDC->Rectangle(x+r-4,y-4,x+r+4,y+4);
	pDC->Rectangle(x-4,y-r-4,x+4,y-r+4);
	pDC->Rectangle(x-4,y+r-4,x+4,y+r+4);

	pDC->SelectObject(pOldPen);     //恢复画笔
	pen.DeleteObject();
	pDC->SelectObject(pOldBrush); //恢复刷子
	brush.DeleteObject();
}

IMPLEMENT_SERIAL(CArc,CObject,1)
	void CArc::Serialize(CArchive& ar)
{
	CCircle::Serialize(ar);
	if (ar.IsStoring())
	{
		ar<<m_Angle1<<m_Angle2;
	} 
	else
	{
		ar>>m_Angle1>>m_Angle2;
	}
}


//**********************************************************************************************************************************


//作用：计算点（xx,yy)到线段（x1,y1)(x2,y2)的距离，返回计算的距离值
double CDraw::PointLine(double xx,double yy,double x1,double y1,double x2,double y2)
{
	double a,b,c,ang1,ang2,ang;
	//计算三条边的距离
	a=CalDisp(x1,y1,xx,yy);
	if(a==0.0)return 0.0;
	b=CalDisp(x2,y2,xx,yy);
	if(b==0.0)return 0.0;
	c=CalDisp(x1,y1,x2,y2);

	//如果（x1,y1)和(x2,y2)的坐标相同，既线段是一个点，退出函数并返回距离
	if(c==0.0) return a;
	
	if(a<b) //如果(xx,yy)的点(x1,y1）这条边较短
	{
		if(y1==y2) //如果直线段AB是水平线，得到直线段AB的弧度
		{
			if(x1<x2)
				ang1=0;
			else
				ang1=(double)pi;
		}
		else  //如果直线段AB不是水平线，得到直线段AB与X轴正向夹角的弧度
		{
			ang1=(double)acos((x2-x1)/c);
			if(y1>y2)ang1=(double)pi*2-ang1;  //直线(x1,y1)-(x2,y2)的弧度
		}
		ang2=(double)acos((xx-x1)/a);
		if(y1>yy)ang2=(double)pi*2-ang2;  //直线(x1,y1)-(xx,yy)的弧度
		ang=ang2-ang1;
		if(ang<0)ang=-ang;
		if(ang>pi) ang=(double)pi*2-ang;  //交角的大小
		if(ang>pi/2) return a;    //如果为钝角，直接返回距离
		else 
			return (a*(double)sin(ang)); //否则返回计算得到的距离 
	}

	else //如果(xx,yy)的点(x2,y2）这条边较短
	{
		if(y1==y2) //如果两个点的纵坐标相同，则直接得到直线斜率的弧度
		{
			if(x1<x2)
				ang1=(double)pi;
			else
				ang1=0;
		}
		else //如果两个点的纵坐标不同
		{
			ang1=(double)acos((x1-x2)/c);     //直线（x2,y2)-(x1,y1)的斜率的弧度
			if(y2>y1)ang1=(double)pi*2-ang1;
		}
		ang2=(double)acos((xx-x2)/b);     //直线（x2,x1）-(xx,yy)的斜率的弧度
		if(y2>yy)ang2=(double)pi*2-ang2;
		ang=ang2-ang1;
		if(ang<0) ang=-ang;
		if(ang>pi) ang=(double)pi*2-ang;  //交角的大小 ?
		if(ang>pi/2)  return b;   //如果为钝角，直接返回距离
		else 
			return(b*(double)sin(ang));  //否则返回计算得到的距离
	}
}

//计算点(x1,y1)与点(x2,y2)间的距离
double CDraw::CalDisp(double x1,double y1,double x2,double y2)
{
	return (double)sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}


//函数PointRgn作用：判断一个点是否在任意多边形区域内
//参数：Numble-多边形顶点数目，x,y是点的坐标，多边形顶点的坐标在结构数组PointList中
//返回：1-点在多边形区域  0-不在
BOOL CDraw::PointRgn(double x,double y,int Numble,PointStruct *PointList,double blc)
{
	//定义一个CRgn类
	CRgn rgn;         
	int x1,y1,i;
	// 定义一个点结构指针
	POINT* pPoint;      

	//如果点的数目<3即不是一个区域返回不成功标志
	if(Numble<3) 
		return 0;

	 //给结构指针动态分配Numble顶点
	pPoint = new POINT[Numble];

	//将按中点的坐标转变成屏幕坐标也就是像素坐标
	x1=(int)(x/blc);
	y1=(int)(y/blc);

	//将封闭区域，就是多边形区域，各点的坐标转成屏幕坐标也就是像素坐标
	for(i=0;i<Numble;i++)
	{
		pPoint[i].x=(int)((PointList[i].x)/blc);
		pPoint[i].y=(int)((PointList[i].y)/blc);
	}

	//初试化一个多边形区域
	rgn.CreatePolygonRgn(pPoint,Numble,1);
	//如果在区域内j=1,否则j=0;
	i=(rgn.PtInRegion(x1,y1)!= 0);//***********************??????????***************
	//删除定义的rgn对象
	rgn.DeleteObject();	
	delete pPoint;
	return i;
}

//这是为了计算直线段和直线段相交关系的函数
int CDraw::LineLine(double m_X1 , double m_Y1 , double m_X2 , double m_Y2, double XXX1 , double YYY1 , double XXX2 , double YYY2)
{
	double xx1,xx2,yy1,yy2;//第一条直线
	double x1,x2,y1,y2,n1,n2,n3,n4;//第二条直线
	double k1 , k2;//两条直线的斜率

	if (m_X1>m_X2)
	{
		xx1 = m_X2;
		xx2 = m_X1;
		yy1 = m_Y2;
		yy2 = m_Y1;
	}
	else
	{
		xx1 = m_X1;
		xx2 = m_X2;
		yy1 = m_Y1;
		yy2 = m_Y2;

	}
	if (fabs(xx1- xx2)<=0.001 && fabs(yy1- yy2)<= 0.001)
	{
		return 0;
	}

	if (XXX1 > XXX2)
	{
		x1 = XXX2;
		x2 = XXX1;
		y1 = YYY2;
		y2 = YYY1;
	}
	else
	{
		x1 = XXX1;
		x2 = XXX2;
		y1 = YYY1;
		y2 = YYY2;
	}
	if (fabs(x1 - x2 )<= 0.001  && fabs( y1- y2)<=0.001)
	{
		return 0;
	}
	if (fabs(x1 - x2)<= 0.001 && fabs( xx1 - xx2)<=0.001 )
	{
		if (x1 != xx1)
		{
			return 0;
		}

		n1 = min(yy1,yy2);
		n2 = max(yy1,yy2);
		n3 = min(y1,y2);
		n4 = max(y1,y2);
		if (n1>n4 || n2<n3)
		{
			return 0;
		}
		else if (n1 == n4 || n2 == n3)
		{
			if (n1 == n4)
			{
				return 1;
			}
			if (n2 == n3)
			{
				return 1;
			}
		}
		else
		{
			return 2;
		}
	}

	else if (fabs(x1 - x2)<= 0.001 && xx1 != xx2)
	{
		if (!(x1>=xx1 && x1<=xx2))
		{
			return 0;
		}

		double nn = yy1+(x1-xx1)/(xx2-xx1)*(yy2-yy1);
		if (nn>=y1 && nn<=y2 || nn>= y2 && nn<=y1)
		{
			return 1;
		}
		else
		{
			return 0 ;
		}
	}
	else if(x1!=x2 && fabs(xx1 - xx2)<=0.001)
	{
		if (!(xx1>=x1 && xx1<=x2))
		{
			return 0;
		}
		double nn1 = y1+(xx1 - x1)/(x2 - x1)*(y2 - y1);
		if (nn1>=yy1 && nn1<=yy2 || nn1>=yy2 && nn1<=yy1)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	else
	{
		k1 = (y2 - y1)/(x2 -x1);
		k2 = (yy2 - yy1)/(xx2 - xx1);
		if (k1 == k2)
		{
			if (x1>xx2 || x2<xx1)
			{
				return 0;
			}
			else
			{
				n1 = y1+ (0-x1)*k1;
				n2 = yy1+ (0-x1)*k2;
				if (n1 !=n2)
				{
					return 0;
				}
				else
				{
					n3 = max(xx1,x1);
					n4 = min(xx2,x2);

					double nn2 = y1+(n3-x1)*k1;
					if (n3 == n4)
					{
						return 1;
					}

					return 2;

				}
			}
		}
		else
		{
			double nn2 = (yy1-y1+x1*k1-xx1*k2)/(k1-k2);
			if ((nn2>=x1 && nn2<=x2) && (nn2>=xx1 && nn2<=xx2))
			{
				return 1;
			}
			else
				return 0;
		}
	}
	return 0;
}

//计算直线段与矩形相交
BOOL CDraw:: LineRan(double xx1 , double yy1 , double xx2 , double yy2,PointStruct * pPoint,double blc)
{
	double x1 ,y1,x2,y2;
	for (int i = 0 ;i < 4 ;i++)
	{
		x1 = pPoint[i].x;
		y1 = pPoint[i].y;
		x2 = pPoint[i+1].x;
		y2 = pPoint[i+1].y;

		//如果相交，LineLine为非0，就返回TRUE
		if (LineLine(xx1,yy1,xx2,yy2,x1,y1,x2,y2))
		{
			return TRUE;
			break;
		}	
	}
	return FALSE;
}

//计算多段线与矩形相交
BOOL CDraw:: PLineRan(int PointNunb,PointStruct * pPLine, PointStruct * pPoint,double blc)
{
	double x1,y1,x2,y2;
	for (int i=0 ; i<PointNunb-1 ; i++)//对多线段的所有直线直段循环
	{
		x1 = pPLine[i].x;
		y1 = pPLine[i].y;
		x2 = pPLine[i+1].x;
		y2 = pPLine[i+1].y;

		//计算多线段中的直线与矩形的相交关系
		if (LineRan(x1,y1,x2,y2,pPoint,blc))
		{
			//如果相交，就返回TRUE,并停止循环
			return TRUE;
			break;
		}
	}
	return FALSE;	
}


//计算线段端点是不是在矩形区域内----光光自制
BOOL CDraw::PointRgnRec(double x ,double y, PointStruct *PointList,double blc)
{
	//定义一个rgn对象
	CRgn rgn;
	int p_x1,p_y1,x1,y1,x2,y2;
	p_x1 = (int)(x/blc);
	p_y1 = (int)(y/blc);
	x1 = (int)((PointList[0].x )/blc) ;
	y1 = (int)((PointList[0].y )/blc) ;
	x2 = (int)((PointList[2].x )/blc) ;
	y2 = (int)((PointList[2].y )/blc) ;

	//初始化一个多边形区域
	rgn.CreateRectRgn(x1,y1,x2,y2);
	//若点在区域内为非零值，否则为0
	BOOL YN = rgn.PtInRegion(p_x1,p_y1);
	//删除定义的rgn对象
	rgn.DeleteObject();
	return YN;
}



//*****************************************************************************************************************
//获取图层颜色
COLORREF CLoadDXF::GetLayerColor(CString Layer)
{
	CMainFrame* pFrame=(CMainFrame*)(AfxGetApp()->m_pMainWnd);
	CFMDoc *pDoc=(CFMDoc *)pFrame->GetActiveDocument();
	int nSize = pDoc->m_LayerArray.GetSize();
	if (nSize<=0)
	{
		AfxMessageBox(_T("没有发现图层数据"));
		return RGB(255,255,255);
	}
	for (int i = 0; i< nSize ;i++)
	{
		if (Layer == pDoc->m_LayerArray.GetAt(i)->m_Layer)
		{
			return pDoc->m_LayerArray.GetAt(i)->m_Color;
		}
	}
	return RGB(255,255,255);
}
//得到图层显示隐藏标志
BOOL CLoadDXF:: GetLayerDelete(CString Layer)
{
	CMainFrame* pFrame=(CMainFrame*)(AfxGetApp()->m_pMainWnd);
	CFMDoc *pDoc=(CFMDoc *)pFrame->GetActiveDocument();
	int nSize = pDoc->m_LayerArray.GetSize();
	if (nSize<=0)
	{
		AfxMessageBox(_T("没有发现图数据"));
		return TRUE;       //b_Delete将是TRUE
	}
	for (int i = 0; i< nSize ;i++)
	{
		if (Layer == pDoc->m_LayerArray.GetAt(i)->m_Layer)
		{
			return pDoc->m_LayerArray.GetAt(i)->b_Delete;
		}

	}
	return TRUE; //b_Delete将是TRUE
}

//RGB转换
COLORREF CLoadDXF::LoadColorreF(int Index)
{
	if(Index>255||Index<0)
	{
		return RGB(255,255,255);
	}
	int r,g,b;
	switch(Index)
	{
	case 1:r=255;g=0;b=0;break;
	case 2:r=255;g=255;b=0;break;
	case 3:r=0;g=255;b=0;break;
	case 4:r=0;g=255;b=255;break;
	case 5:r=0;g=0;b=255;break;
	case 6:r=255;g=0;b=255;break;
	case 7:r=255;g=255;b=255;break;
	case 8:r=128;g=128;b=128;break;
	case 9:r=192;g=192;b=192;break;
	case 10:r=255;g=0;b=0;break;
	case 11:r=255;g=127;b=127;break;
	case 12:r=204;g=0;b=0;break;
	case 13:r=204;g=102;b=102;break;
	case 14:r=153;g=0;b=0;break;
	case 15:r=153;g=76;b=76;break;
	case 16:r=127;g=0;b=0;break;
	case 17:r=127;g=63;b=63;break;
	case 18:r=76;g=0;b=0;break;
	case 19:r=76;g=38;b=38;break;
	case 20:r=255;g=63;b=0;break;
	case 21:r=255;g=159;b=127;break;
	case 22:r=204;g=51;b=0;break;
	case 23:r=204;g=127;b=102;break;
	case 24:r=153;g=38;b=0;break;
	case 25:r=153;g=95;b=76;break;
	case 26:r=127;g=31;b=0;break;
	case 27:r=127;g=79;b=63;break;
	case 28:r=76;g=19;b=0;break;
	case 29:r=76;g=47;b=38;break;
	case 30:r=255;g=127;b=0;break;
	case 31:r=255;g=191;b=127;break;
	case 32:r=204;g=102;b=0;break;
	case 33:r=204;g=153;b=102;break;
	case 34:r=153;g=76;b=0;break;
	case 35:r=153;g=114;b=76;break;
	case 36:r=127;g=63;b=0;break;
	case 37:r=127;g=95;b=63;break;
	case 38:r=76;g=38;b=0;break;
	case 39:r=76;g=57;b=38;break;
	case 40:r=255;g=191;b=0;break;
	case 41:r=255;g=223;b=127;break;
	case 42:r=204;g=153;b=0;break;
	case 43:r=204;g=178;b=102;break;
	case 44:r=153;g=114;b=0;break;
	case 45:r=153;g=133;b=76;break;
	case 46:r=127;g=95;b=0;break;
	case 47:r=127;g=111;b=63;break;
	case 48:r=76;g=57;b=0;break;
	case 49:r=76;g=66;b=38;break;
	case 50:r=255;g=255;b=0;break;
	case 51:r=255;g=255;b=127;break;
	case 52:r=204;g=204;b=0;break;
	case 53:r=204;g=204;b=102;break;
	case 54:r=153;g=153;b=0;break;
	case 55:r=153;g=153;b=76;break;
	case 56:r=127;g=127;b=0;break;
	case 57:r=127;g=127;b=63;break;
	case 58:r=76;g=76;b=0;break;
	case 59:r=76;g=76;b=38;break;
	case 60:r=191;g=255;b=0;break;
	case 61:r=223;g=255;b=127;break;
	case 62:r=153;g=204;b=0;break;
	case 63:r=178;g=204;b=102;break;
	case 64:r=114;g=153;b=0;break;
	case 65:r=133;g=153;b=76;break;
	case 66:r=95;g=127;b=0;break;
	case 67:r=111;g=127;b=63;break;
	case 68:r=57;g=76;b=0;break;
	case 69:r=66;g=76;b=38;break;
	case 70:r=127;g=255;b=0;break;
	case 71:r=191;g=255;b=127;break;
	case 72:r=102;g=204;b=0;break;
	case 73:r=153;g=204;b=102;break;
	case 74:r=76;g=153;b=0;break;
	case 75:r=114;g=153;b=76;break;
	case 76:r=63;g=127;b=0;break;
	case 77:r=95;g=127;b=63;break;
	case 78:r=38;g=76;b=0;break;
	case 79:r=57;g=76;b=38;break;
	case 80:r=63;g=255;b=0;break;
	case 81:r=159;g=255;b=127;break;
	case 82:r=51;g=204;b=0;break;
	case 83:r=127;g=204;b=102;break;
	case 84:r=38;g=153;b=0;break;
	case 85:r=95;g=153;b=76;break;
	case 86:r=31;g=127;b=0;break;
	case 87:r=79;g=127;b=63;break;
	case 88:r=19;g=76;b=0;break;
	case 89:r=47;g=76;b=38;break;
	case 90:r=0;g=255;b=0;break;
	case 91:r=127;g=255;b=127;break;
	case 92:r=0;g=204;b=0;break;
	case 93:r=102;g=204;b=102;break;
	case 94:r=0;g=153;b=0;break;
	case 95:r=76;g=153;b=76;break;
	case 96:r=0;g=127;b=0;break;
	case 97:r=63;g=127;b=63;break;
	case 98:r=0;g=76;b=0;break;
	case 99:r=38;g=76;b=38;break;
	case 100:r=0;g=255;b=63;break;
	case 101:r=127;g=255;b=159;break;
	case 102:r=0;g=204;b=51;break;
	case 103:r=102;g=204;b=127;break;
	case 104:r=0;g=153;b=38;break;
	case 105:r=76;g=153;b=95;break;
	case 106:r=0;g=127;b=31;break;
	case 107:r=63;g=127;b=79;break;
	case 108:r=0;g=76;b=19;break;
	case 109:r=38;g=76;b=47;break;
	case 110:r=0;g=255;b=127;break;
	case 111:r=127;g=255;b=191;break;
	case 112:r=0;g=204;b=102;break;
	case 113:r=102;g=204;b=153;break;
	case 114:r=0;g=153;b=76;break;
	case 115:r=76;g=153;b=114;break;
	case 116:r=0;g=127;b=63;break;
	case 117:r=63;g=127;b=95;break;
	case 118:r=0;g=76;b=38;break;
	case 119:r=38;g=76;b=57;break;
	case 120:r=0;g=255;b=191;break;
	case 121:r=127;g=255;b=223;break;
	case 122:r=0;g=204;b=153;break;
	case 123:r=102;g=204;b=178;break;
	case 124:r=0;g=153;b=114;break;
	case 125:r=76;g=153;b=133;break;
	case 126:r=0;g=127;b=95;break;
	case 127:r=63;g=127;b=111;break;
	case 128:r=0;g=76;b=57;break;
	case 129:r=38;g=76;b=66;break;
	case 130:r=0;g=255;b=255;break;
	case 131:r=127;g=255;b=255;break;
	case 132:r=0;g=204;b=204;break;
	case 133:r=102;g=204;b=204;break;
	case 134:r=0;g=153;b=153;break;
	case 135:r=76;g=153;b=153;break;
	case 136:r=0;g=127;b=127;break;
	case 137:r=63;g=127;b=127;break;
	case 138:r=0;g=76;b=76;break;
	case 139:r=38;g=76;b=76;break;
	case 140:r=0;g=191;b=255;break;
	case 141:r=127;g=223;b=255;break;
	case 142:r=0;g=153;b=204;break;
	case 143:r=102;g=178;b=204;break;
	case 144:r=0;g=114;b=153;break;
	case 145:r=76;g=133;b=153;break;
	case 146:r=0;g=95;b=127;break;
	case 147:r=63;g=111;b=127;break;
	case 148:r=0;g=57;b=76;break;
	case 149:r=38;g=66;b=76;break;
	case 150:r=0;g=127;b=255;break;
	case 151:r=127;g=191;b=255;break;
	case 152:r=0;g=102;b=204;break;
	case 153:r=102;g=153;b=204;break;
	case 154:r=0;g=76;b=153;break;
	case 155:r=76;g=114;b=153;break;
	case 156:r=0;g=63;b=127;break;
	case 157:r=63;g=95;b=127;break;
	case 158:r=0;g=38;b=76;break;
	case 159:r=38;g=57;b=76;break;
	case 160:r=0;g=63;b=255;break;
	case 161:r=127;g=159;b=255;break;
	case 162:r=0;g=51;b=204;break;
	case 163:r=102;g=127;b=204;break;
	case 164:r=0;g=38;b=153;break;
	case 165:r=76;g=95;b=153;break;
	case 166:r=0;g=31;b=127;break;
	case 167:r=63;g=79;b=127;break;
	case 168:r=0;g=19;b=76;break;
	case 169:r=38;g=47;b=76;break;
	case 170:r=0;g=0;b=255;break;
	case 171:r=127;g=127;b=255;break;
	case 172:r=0;g=0;b=204;break;
	case 173:r=102;g=102;b=204;break;
	case 174:r=0;g=0;b=153;break;
	case 175:r=76;g=76;b=153;break;
	case 176:r=0;g=0;b=127;break;
	case 177:r=63;g=63;b=127;break;
	case 178:r=0;g=0;b=76;break;
	case 179:r=38;g=38;b=76;break;
	case 180:r=63;g=0;b=255;break;
	case 181:r=159;g=127;b=255;break;
	case 182:r=51;g=0;b=204;break;
	case 183:r=127;g=102;b=204;break;
	case 184:r=38;g=0;b=153;break;
	case 185:r=95;g=76;b=153;break;
	case 186:r=31;g=0;b=127;break;
	case 187:r=79;g=63;b=127;break;
	case 188:r=19;g=0;b=76;break;
	case 189:r=47;g=38;b=76;break;
	case 190:r=127;g=0;b=255;break;
	case 191:r=191;g=127;b=255;break;
	case 192:r=102;g=0;b=204;break;
	case 193:r=153;g=102;b=204;break;
	case 194:r=76;g=0;b=153;break;
	case 195:r=114;g=76;b=153;break;
	case 196:r=63;g=0;b=127;break;
	case 197:r=95;g=63;b=127;break;
	case 198:r=38;g=0;b=76;break;
	case 199:r=57;g=38;b=76;break;
	case 200:r=191;g=0;b=255;break;
	case 201:r=223;g=127;b=255;break;
	case 202:r=153;g=0;b=204;break;
	case 203:r=178;g=102;b=204;break;
	case 204:r=114;g=0;b=153;break;
	case 205:r=133;g=76;b=153;break;
	case 206:r=95;g=0;b=127;break;
	case 207:r=111;g=63;b=127;break;
	case 208:r=57;g=0;b=76;break;
	case 209:r=66;g=38;b=76;break;
	case 210:r=255;g=0;b=255;break;
	case 211:r=255;g=127;b=255;break;
	case 212:r=204;g=0;b=204;break;
	case 213:r=204;g=102;b=204;break;
	case 214:r=153;g=0;b=153;break;
	case 215:r=153;g=76;b=153;break;
	case 216:r=127;g=0;b=127;break;
	case 217:r=127;g=63;b=127;break;
	case 218:r=76;g=0;b=76;break;
	case 219:r=76;g=38;b=76;break;
	case 220:r=255;g=0;b=191;break;
	case 221:r=255;g=127;b=223;break;
	case 222:r=204;g=0;b=153;break;
	case 223:r=204;g=102;b=178;break;
	case 224:r=153;g=0;b=114;break;
	case 225:r=153;g=76;b=133;break;
	case 226:r=127;g=0;b=95;break;
	case 227:r=127;g=63;b=111;break;
	case 228:r=76;g=0;b=57;break;
	case 229:r=76;g=38;b=66;break;
	case 230:r=255;g=0;b=127;break;
	case 231:r=255;g=127;b=191;break;
	case 232:r=204;g=0;b=102;break;
	case 233:r=204;g=102;b=153;break;
	case 234:r=153;g=0;b=76;break;
	case 235:r=153;g=76;b=114;break;
	case 236:r=127;g=0;b=63;break;
	case 237:r=127;g=63;b=95;break;
	case 238:r=76;g=0;b=38;break;
	case 239:r=76;g=38;b=57;break;
	case 240:r=255;g=0;b=63;break;
	case 241:r=255;g=127;b=159;break;
	case 242:r=204;g=0;b=51;break;
	case 243:r=204;g=102;b=127;break;
	case 244:r=153;g=0;b=38;break;
	case 245:r=153;g=76;b=95;break;
	case 246:r=127;g=0;b=31;break;
	case 247:r=127;g=63;b=79;break;
	case 248:r=76;g=0;b=19;break;
	case 249:r=76;g=38;b=47;break;
	case 250:r=51;g=51;b=51;break;
	case 251:r=91;g=91;b=91;break;
	case 252:r=132;g=132;b=132;break;
	case 253:r=173;g=173;b=173;break;
	case 254:r=214;g=214;b=214;break;
	case 255:r=255;g=255;b=255;break;
	}
	return RGB(r,g,b);
}