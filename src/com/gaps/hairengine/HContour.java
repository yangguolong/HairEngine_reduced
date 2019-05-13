package com.gaps.hairengine;

import java.util.Arrays;

public class HContour 
{
	public int		contourNum = 0;
	public int[]	contourPointNum = null;
	public HPoint[]	contourPoints = null;
	@Override
	public String toString() {
		return "HContour [contourNum=" + contourNum + ", contourPointNum="
				+ Arrays.toString(contourPointNum) + ", contourPoints="
				+ Arrays.toString(contourPoints) + "]";
	}
	
	
}
