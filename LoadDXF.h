#pragma once

#ifndef LOADDXF_H
#define LOADDXF_H

#include "dl_creationadapter.h"

//多段线顶点坐标
typedef struct  
{
	double x;
	double y;
	double z;
}PointStruct;

//*************************************************************************************************
class CLoadDXF : public DL_CreationAdapter
{
public:
	CLoadDXF();
	~CLoadDXF();
	virtual void addLayer(const DL_LayerData& data);
	virtual void addLine(const DL_LineData& data);
	virtual void addArc(const DL_ArcData& data);
	virtual void addCircle(const DL_CircleData& data);
	virtual void addPolyline(const DL_PolylineData& data);
	virtual void addVertex(const DL_VertexData& data);

	//RGB转换函数  
	COLORREF LoadColorreF(int Index);
	//获取图层颜色
	COLORREF GetLayerColor(CString Layer);
	//确认图层是否隐藏，得到Delete
	BOOL GetLayerDelete(CString Layer);

private:
	int j;
	int Fill;     //多段线读取用
	int Numble;
	PointStruct* m_PointList;
};

#endif
//******************************************************************************************************

//图形元素基类
class CDraw : public CObject
{
protected:
	CString m_Layer;
	COLORREF m_Color;
	BOOL b_Delete;
	virtual void Serialize(CArchive& ar); 
public:
	CDraw(){}
	CDraw(CString Layer,COLORREF Color,BOOL Delete)
	{
		m_Layer = Layer;
		m_Color = Color;
		b_Delete = Delete;
	}
	//绘制图形虚函数
	virtual void Draw(CDC *pDC,int m_DrawMode,int m_DrawMode1,COLORREF BackColor) = 0;
	//图形元素边界矩形计算虚函数
	virtual void GetRect(double *minX,double *minY,double *maxX,double *maxY) = 0;
	//计算点到直线的距离的函数
	double PointLine(double xx,double yy,double x1,double y1,double x2,double y2);
	//计算两点间的距离的函数
	double CalDisp(double x1,double y1,double x2,double y2);
	//判断点是否在一个多边形区域中的函数
	BOOL PointRgn(double x,double y,int Numble,PointStruct *PointList,double blc);
	//判断是否被点选中的函数
	virtual BOOL IsPoint(double x,double y,double j1,double blc) = 0;	
	//图形的旋转
	void RotatePoint (double baseX , double baseY , double angle , double *xx, double *yy);
	//图形的旋转中的函数
	double GetAngle (double baseX , double baseY , double xx , double yy);
	//为了实现各类图形元素的计算,搞这个虚函数
	virtual void Rotate (double baseX , double baseY , double angle) = 0;
	//图形缩放中的子函数
	void ScalePoint (double x_Base , double y_Base , double x_Scale , double y_Scale , double* xx , double* yy);
	//计算图形元素放缩操作的纯虚函数
	virtual void Scale (double x_Base , double y_Base , double x_Scale , double y_Scale ) = 0;
	//这是图形移动的虚函数
	virtual void Move (double x_Move , double y_Move) = 0;
	//这是为了计算直线段和直线段相交关系的函数
	int LineLine(double m_X1 , double m_Y1 , double m_X2 , double m_Y2, double XXX1 , double YYY1 , double XXX2 , double YYY2);
	//计算直线段与矩形相交
	BOOL LineRan(double xx1 , double yy1 , double xx2 , double yy2,PointStruct * pPoint,double blc);
	//计算多段线与矩形相交
	BOOL PLineRan(int PointNunb,PointStruct * pPLine, PointStruct * pPoint,double blc);
	//计算各图形元素是不是在矩形区域内----光光自制
	BOOL PointRgnRec(double x ,double y, PointStruct *PointList,double blc);
	//框选图形元素判断的虚函数
	virtual BOOL IsRectangle(PointStruct *pPoint ,double blc) = 0;
	//工具栏组合框选择图层
	virtual BOOL IsLayerCombox(CString Lyaer) =0 ;
	//元素顶点的蓝色方框
	virtual void BlueBox(CDC* pDC)=0;

	virtual ~CDraw(){}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//图层类
class CLayer : public CObject 
{
protected:
	DECLARE_SERIAL(CLayer);
	CString m_Layer;
	COLORREF m_Color;
	BOOL b_Delete;
public:
	CLayer(){}
	CLayer(CString Layer,COLORREF Color,BOOL Delete)
	{
		m_Layer = Layer;
		m_Color = Color;
		b_Delete = Delete;
	}
	virtual void Serialize(CArchive& ar);
};

//直线类////////////////////////////////////////////////////////////////////////////////////////////////////////
class CLine :public CDraw
{
protected:
	DECLARE_SERIAL(CLine);
	double m_X1,m_X2,m_Y1,m_Y2;
public:
	CLine(){}
	CLine(CString Layer,COLORREF Color,BOOL Delete,double X1,double Y1,double X2,double Y2)
		:CDraw(Layer,Color,Delete)
	{
		m_X1 = X1;
		m_Y1 = Y1;
		m_X2 = X2;
		m_Y2 = Y2;
	}
	virtual void Draw(CDC *pDC,int m_DrawMode,int m_DrawMode1,COLORREF BackColor);
	virtual void GetRect(double *minX,double *minY,double *maxX,double *maxY);
	virtual void Serialize(CArchive& ar);
	virtual BOOL IsPoint(double x,double y,double j1,double blc);
	virtual void Rotate (double baseX , double baseY , double angle);
	virtual void Scale (double x_Base , double y_Base , double x_Scale , double y_Scale );
	virtual void Move (double x_Move , double y_Move) ;
	virtual BOOL IsRectangle(PointStruct *pPoint ,double blc) ;
	virtual BOOL IsLayerCombox(CString Lyaer);
	virtual void BlueBox(CDC* pDC);
};

//多段线///////////////////////////////////////////////////////////////////////////////////////////////////
class CPline :public CDraw
{
protected:
	int m_Numble; //顶点数
	BOOL b_Fill;//多线段标志，1=关闭，0=开
	PointStruct* m_PointList;//存储顶点坐标的数组指针
	DECLARE_SERIAL(CPline);
public:
	CPline()
	{m_Numble = 0;}
	CPline(CString Layer,COLORREF Color,BOOL Delete,int Numble,PointStruct *PointList,BOOL Fill)
		:CDraw(Layer,Color,Delete)
	{
		m_Numble = Numble;
		b_Fill = Fill;
		m_PointList = new PointStruct[Numble+1];//动态分配存储顶点坐标的结构数组
		if (Numble > 0)                          //如果顶点数目大于0
		{
			for (int i = 0 ;i < Numble ; i++)
				m_PointList[i] = PointList[i];
		}
	}

	virtual void Draw(CDC *pDC,int m_DrawMode,int m_DrawMode1,COLORREF BackColor);
	virtual void GetRect(double *minX,double *minY,double *maxX,double *maxY);
	virtual void Serialize(CArchive& ar);
	virtual BOOL IsPoint(double x,double y,double j1,double blc);	
	virtual void Rotate (double baseX , double baseY , double angle);
	virtual void Scale (double x_Base , double y_Base , double x_Scale , double y_Scale );
	virtual void Move (double x_Move , double y_Move) ;
	virtual BOOL IsRectangle(PointStruct *pPoint ,double blc) ;
	virtual BOOL IsLayerCombox(CString Lyaer);
	virtual void BlueBox(CDC* pDC);

	~CPline()
	{
		if (m_Numble>0)           //如果顶点数目大于0，删除动态分配的结构数组
			delete m_PointList;
	}
};

//圆类//////////////////////////////////////////////////////////////////////////////////////////////////////////
class CCircle:public CDraw
{
protected:
	double m_CirceX , m_CirceY , m_CirceR;
	DECLARE_SERIAL(CCircle);
public:
	CCircle(){}
	CCircle(CString Layer,COLORREF Color,BOOL Delete,double CirceX,double CirceY,double CirceR)
		:CDraw(Layer,Color,Delete)
	{
		m_CirceX = CirceX;
		m_CirceY = CirceY;
		m_CirceR = CirceR;
	}
	virtual void Draw(CDC *pDC,int m_DrawMode,int m_DrawMode1,COLORREF BackColor);
	virtual void GetRect(double *minX,double *minY,double *maxX,double *maxY);
	virtual void Serialize(CArchive& ar);
	virtual BOOL IsPoint(double x,double y,double j1,double blc);	
	virtual void Rotate (double baseX , double baseY , double angle);
	virtual void Scale (double x_Base , double y_Base , double x_Scale , double y_Scale );
	virtual void Move (double x_Move , double y_Move) ;
	virtual BOOL IsRectangle(PointStruct *pPoint ,double blc) ;
	virtual BOOL IsLayerCombox(CString Lyaer);
	virtual void BlueBox(CDC* pDC);
};

//弧线类/////////////////////////////////////////////////////////////////////////////////////////////////////////
class CArc:public CCircle
{
protected:
	//圆弧的起点和终点角度（用弧度表示）
	double m_Angle1,m_Angle2;
	BOOL IsInArc(double angle);
	DECLARE_SERIAL(CArc);
public:
	CArc(){}
	CArc(CString Layer,COLORREF Color,BOOL Delete,double CirceX,double CirceY,double CirceR,double Angle1,double Angle2)
		:CCircle(Layer,Color,Delete,CirceX,CirceY,CirceR)
	{
		m_Angle1 = Angle1;
		m_Angle2 = Angle2;
	}
	virtual void Draw(CDC *pDC,int m_DrawMode,int m_DrawMode1,COLORREF BackColor);
	virtual void GetRect(double *minX,double *minY,double *maxX,double *maxY);
	virtual void Serialize(CArchive& ar);
	virtual BOOL IsPoint(double x,double y,double j1,double blc);	
	virtual void Rotate (double baseX , double baseY , double angle);
	virtual void Scale (double x_Base , double y_Base , double x_Scale , double y_Scale );
	//virtual void Move (double x_Move , double y_Move);
	//virtual BOOL IsRectangle(PointStruct *pPoint ,double blc) ;
	virtual BOOL IsLayerCombox(CString Lyaer);
	virtual void BlueBox(CDC* pDC);
};