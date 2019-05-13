
package com.gaps.hairengine;


import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Bitmap;
import android.util.Log;

public class HairEngine
{
	private Context m_context;
		
	public boolean initEngine(String dataDir, boolean usingRegressor, Context context)
	{
		m_context = context;

		_ObjHandle = initEngineHE(dataDir, usingRegressor);
		if(_ObjHandle != 0)
			return true;
		else
			return false;
	}
	public boolean setImage(Bitmap img, String workDir)
	{
		return setImageHE(_ObjHandle, img, workDir);
	}
	
	public HContour addStroke(int type, int size, HPoint[] points)
	{
		int arrayIndex = 0;
		int length = points.length;
		float [] pointsF = new float[length *2];
		for(int i=0; i<length; i++)
		{
			pointsF[arrayIndex++] = points[i].x;
			pointsF[arrayIndex++] = points[i].y;
		}
		
		float [] rstData = addStrokeHE(_ObjHandle, type, size, pointsF);
		arrayIndex = 0;
		HContour rstContour = new HContour();
		if(rstData == null)
		{
			rstContour.contourNum = 0;
		}
		else
		{
			rstContour.contourNum = (int)(rstData[arrayIndex++]);
			rstContour.contourPointNum = new int[rstContour.contourNum];
			int totalNum = 0;
			for(int i=0; i<rstContour.contourNum; i++)
			{
				int tmpNum = (int)(rstData[arrayIndex++]);
				rstContour.contourPointNum[i] = tmpNum;
				totalNum += tmpNum;
			}
			rstContour.contourPoints = new HPoint[totalNum];
			for(int i=0; i<totalNum; i++)
				rstContour.contourPoints[i] = new HPoint();
			for(int i=0; i<totalNum; i++)
			{
				rstContour.contourPoints[i].x = rstData[arrayIndex++];
				rstContour.contourPoints[i].y = rstData[arrayIndex++];
			}
		}
		
		return rstContour;
	}
	
	public boolean clearStroke()
	{
		return clearStrokeHE(_ObjHandle);
	}
	
	public boolean finishStroke()
	{
		return finishStrokeHE(_ObjHandle);
	}
	public boolean finishStrokeStep1()
	{
		return finishStrokeStep1HE(_ObjHandle);
	}
	
	public boolean finishStrokeStep2()
	{
		return finishStrokeStep2HE(_ObjHandle);
	}
	
	public boolean initViewer(int width, int height)
	{
		return initViewerHE(_ObjHandle, width, height);
	}
	
	public boolean closeViewer()
	{
		return closeViewerHE(_ObjHandle);
	}
	
	public boolean resizeViewer(int width, int height)
	{
		return resizeViewerHE(_ObjHandle, width, height);
	}
	
	public boolean setHairDir(String hairDir)
	{
		return setHairDirHE(_ObjHandle, hairDir);
	}
	
	public boolean setTransform(float xAngle, float yAngle, float scale)
	{
		return setTransformHE(_ObjHandle, xAngle, yAngle, scale);
	}
	
	public boolean resetTransform()
	{
		return resetTransformHE(_ObjHandle);
	}
	
	public boolean adjustHairPosition(float disX, float disY, float disZ, float scale)
	{
		return adjustHairPositionHE(_ObjHandle, disX, disY, disZ, scale);
	}
	
	public boolean resetHairPosition()
	{
		return resetHairPositionHE(_ObjHandle);
	}
	
	public boolean enableShadow(boolean enable)
	{
		return enableShadowHE(_ObjHandle, enable);
	}
	
	//need to enable expression first, disable expression will reset the expression
	public boolean enableExpression(boolean enable)
	{
		return enableExpressionHE(_ObjHandle, enable);
	}
	
	//expression 0 angle, 1 smile close
	public boolean changeExpression(int expressionID)
	{
		return changeExpressionHE(_ObjHandle, expressionID);
	}
	
	//modifier 0 - 12
	public boolean changeModifier(int modifierID)
	{
		return changeModifierHE(_ObjHandle, modifierID);
	}
	
	public boolean changeHairColor(float r, float g, float b, float a)
	{
		return changeHairColorHE(_ObjHandle, r, g, b, a);
	}
	
	public boolean resetExpression()
	{
		return resetExpressionHE(_ObjHandle);
	}
	
	public boolean render()
	{
		return renderHE(_ObjHandle);
	}
	
	public boolean setDermabrasionDegree(int degree)
	{
		return setDermabrasionDegreeHE(_ObjHandle, degree);
	}
	
	private int _ObjHandle = 0;
	private native int initEngineHE(String dataDir, boolean usingRegressor);
	private native boolean setImageHE(int objHandle, Bitmap img, String workDir);
	private native float[] addStrokeHE(int objHandle, int type, int size, float[] points);
	private native boolean clearStrokeHE(int objHandle);
	private native boolean finishStrokeHE(int objHandle);
	private native boolean finishStrokeStep1HE(int objHandle);
	private native boolean finishStrokeStep2HE(int objHandle);
	private native boolean initViewerHE(int objHandle, int width, int height);
	private native boolean closeViewerHE(int objHandle);
	private native boolean resizeViewerHE(int objHandle, int width, int height);
	private native boolean setHairDirHE(int objHandle, String hairDir);
	private native boolean setTransformHE(int objHandle, float xAngle, float yAngle, float scale);
	private native boolean resetTransformHE(int objHandle);
	private native boolean adjustHairPositionHE(int objHandle, float disX, float disY, float disZ, float scale);
	private native boolean resetHairPositionHE(int objHandle);
	private native boolean enableShadowHE(int objHandle, boolean enable);
	private native boolean renderHE(int objHandle);
	private native boolean setDermabrasionDegreeHE(int objHandle, int degree);
	
	private native boolean enableExpressionHE(int objHandle, boolean enable);
	private native boolean changeExpressionHE(int objHandle, int expressionID);
	private native boolean changeModifierHE(int objHandle, int modifierID);
	private native boolean changeHairColorHE(int objHandle, float r, float g, float b, float a);
	private native boolean resetExpressionHE(int objHandle);
	
	static
	{
		System.loadLibrary("HairEngineJni");
	}
}
