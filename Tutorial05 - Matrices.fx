//**************************************************************************//
// This is really where we kick off, and is a modified version of the		//
// Microsoft sample code.													//
//																			//
// this shader is written in the ".fx" style; the vertes and pixel shader	//
// are contained in the same file.  										//
//																			//
// BUT this isn't really an effect file as it has no technique.  In the		//
// future we split this FX file into a vertex shader and pixel shader.   	//
//**************************************************************************//

//**************************************************************************//
// Modifications to the MS sample code is copyright of Dr Nigel Barlow,		//
// lecturer in computing, University of Plymouth, UK.						//
// email: nigel@soc.plymouth.ac.uk.											//
//																			//
// You may use, modify and distribute this (rather cack-handed in places)	//
// code subject to the following conditions:								//
//																			//
//	1:	You may not use it, or sell it, or use it in any adapted form for	//
//		financial gain, without my written premission.						//
//																			//
//	2:	You must not remove the copyright messages.							//
//																			//
//	3:	You should correct at least 10% of the typig abd spekking errirs.   //
//**************************************************************************//
//--------------------------------------------------------------------------------------
// File: Tutorial05 - Matrices.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------



//**************************************************************************//
// Constant Buffer Variables.  The world, view and projection matrices.		//
// Just to confuse you, the definition "matrix" or "float4x4" seem to be	//
// identical.																//
//																			//
// This structure must be consistent with the definition you use in the		//
// program that refers to this shader.  So much for encapsulation; Roy		//
// Tucker (Comp Sci students will know him) will not approve.				//
//**************************************************************************//
cbuffer ConstantBufferMatrices : register( b0 )
{
	matrix MatWorld;
	matrix MatView;
	matrix MatProjection;
}



//**************************************************************************//
// Vertex shader input structure.	The semantics (the things after the		//
// colon) look a little weird.  The semantics are used (so Microsoft tell	//
// us) used by the compiler to link shader inputs and outputs. 				//
//																			//
// For this to work, you must ensure that the vertex structure you use in	//
// any program that uses this shader is the same as below, vertex position,	//
// and colour, in that order!												//
//**************************************************************************//
struct VS_INPUT
{
    float4 Pos   : POSITION;
    float4 Color : COLOR;
};


//**************************************************************************//
// Pixel shader input structure.	Just a position and a colour.			//
//																			//
// NOTE: Pos has a different samentic to the structure above; get it wrong	//
// and nothing works.  That's because the pixel shader is in a different	//
// stage in the rendering pipeline.											//
//**************************************************************************//
struct PS_INPUT
{
    float4 Pos   : SV_POSITION;		
    float4 Color : COLOR;
};


//**************************************************************************//
// Vertex shader.  This one isn't doing any shading, it is simply			//
// multiplying each vertex by the WVP (World * View * Projection) matrix.	//
//**************************************************************************//
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( input.Pos, MatWorld );		//I think it is bad practice
    output.Pos = mul( output.Pos, MatView );		//to multiply the 3 matrices together
    output.Pos = mul( output.Pos, MatProjection );	//here.  I think it would be better to.
    output.Color = input.Color;						//pass world*view*proj as a single 
													//variable (why??) - Nigel
    return output;
}




//**************************************************************************//
// The pixel shader. There isn't much obviously going on here, we just		//
// extract the colour from the imput and return it.							//
//																			//
// But there is clever stuff going on that's hidden from us.  Each vertex of//
// each cube is defined as a seperate colour.  The shader automatically     //
// interpolates (blends if you wish) the colour between vertices.			//
//**************************************************************************//
float4 PS( PS_INPUT input) : SV_Target
{
    return input.Color;
}
