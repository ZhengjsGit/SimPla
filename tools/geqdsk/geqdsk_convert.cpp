/**
 * @file geqdsk_to_mesh.cpp
 * @author salmon
 * @date 2016-05-17.
 */




#include <oce/BRep_Tool.hxx>

#include <oce/BRepAlgoAPI_Fuse.hxx>

#include <oce/BRepBuilderAPI_MakeEdge.hxx>
#include <oce/BRepBuilderAPI_MakeFace.hxx>
#include <oce/BRepBuilderAPI_MakeWire.hxx>
#include <oce/BRepBuilderAPI_Transform.hxx>

#include <oce/BRepFilletAPI_MakeFillet.hxx>

#include <oce/BRepLib.hxx>

#include <oce/BRepOffsetAPI_MakeThickSolid.hxx>
#include <oce/BRepOffsetAPI_ThruSections.hxx>

#include <oce/BRepPrimAPI_MakeCylinder.hxx>
#include <oce/BRepPrimAPI_MakePrism.hxx>

#include <oce/GC_MakeArcOfCircle.hxx>
#include <oce/GC_MakeSegment.hxx>

#include <oce/GCE2d_MakeSegment.hxx>

#include <oce/gp.hxx>
#include <oce/gp_Ax1.hxx>
#include <oce/gp_Ax2.hxx>
#include <oce/gp_Ax2d.hxx>
#include <oce/gp_Dir.hxx>
#include <oce/gp_Dir2d.hxx>
#include <oce/gp_Pnt.hxx>
#include <oce/gp_Pnt2d.hxx>
#include <oce/gp_Trsf.hxx>
#include <oce/gp_Vec.hxx>

#include <oce/Geom_CylindricalSurface.hxx>
#include <oce/Geom_Plane.hxx>
#include <oce/Geom_Surface.hxx>
#include <oce/Geom_TrimmedCurve.hxx>

#include <oce/Geom2d_Ellipse.hxx>
#include <oce/Geom2d_TrimmedCurve.hxx>

#include <oce/TopExp_Explorer.hxx>

#include <oce/TopoDS.hxx>
#include <oce/TopoDS_Edge.hxx>
#include <oce/TopoDS_Face.hxx>
#include <oce/TopoDS_Wire.hxx>
#include <oce/TopoDS_Shape.hxx>
#include <oce/TopoDS_Compound.hxx>

#include <oce/TopTools_ListOfShape.hxx>

#include <oce/TopoDS_Shape.hxx>
#include <oce/BRepPrimAPI_MakeSphere.hxx>
#include <oce/TDocStd_Document.hxx>
#include <oce/Handle_TDocStd_Document.hxx>
#include <oce/XCAFApp_Application.hxx>
#include <oce/Handle_XCAFApp_Application.hxx>
#include <oce/XCAFDoc_ShapeTool.hxx>
#include <oce/Handle_XCAFDoc_ShapeTool.hxx>
#include <oce/XCAFDoc_DocumentTool.hxx>
#include <oce/STEPCAFControl_Writer.hxx>
#include <oce/STEPCAFControl_Writer.hxx>
#include <oce/XCAFDoc_DocumentTool.hxx>
#include <oce/XCAFApp_Application.hxx>
#include <oce/GeomAPI_PointsToBSpline.hxx>
#include <oce/TColgp_Array1OfPnt.hxx>
#include <oce/Geom_BSplineCurve.hxx>
#include <oce/GeomAPI_Interpolate.hxx>
#include <oce/TColgp_HArray1OfPnt.hxx>

#include <oce/AIS.hxx>
#include <oce/AIS_Shape.hxx>
#include <oce/AIS_InteractiveContext.hxx>
#include <oce/Graphic3d_AspectLine3d.hxx>
#include <oce/Graphic3d_AspectMarker3d.hxx>
#include <oce/Graphic3d_AspectFillArea3d.hxx>
#include <oce/Graphic3d_AspectText3d.hxx>
#include <oce/Graphic3d_GraphicDriver.hxx>
#include <oce/OpenGl_GraphicDriver.hxx>
#include <oce/V3d.hxx>
#include <oce/V3d_View.hxx>
#include <oce/V3d_Viewer.hxx>


#include <oce/BRepPrimAPI_MakeBox.hxx>
#include <oce/Standard_Real.hxx>


TopoDS_Shape
MakeBottle(const Standard_Real myWidth, const Standard_Real myHeight,
           const Standard_Real myThickness)
{

    BRepBuilderAPI_MakeWire wireMaker;

    // Profile : Define Support Points
    {
        Handle(TColgp_HArray1OfPnt) gp_array = new TColgp_HArray1OfPnt(1, 4);
        gp_Pnt p0(-myWidth / 2., 0, 0);
        gp_Pnt p4(myWidth / 2., 0, 0);

//    gp_array->SetValue(0, p0);
        gp_array->SetValue(1, gp_Pnt(-myWidth / 2., -myThickness / 4., 0));
        gp_array->SetValue(2, gp_Pnt(0, -myThickness / 2., 0));
        gp_array->SetValue(3, gp_Pnt(myWidth / 2., -myThickness / 4., 0));
        gp_array->SetValue(4, p4);

        GeomAPI_Interpolate sp(gp_array, true, 1.0e-3);
        sp.Perform();

        wireMaker.Add(BRepBuilderAPI_MakeEdge(sp.Curve()));
    }
    // Body : Prism the Profile
    TopoDS_Face myFaceProfile = BRepBuilderAPI_MakeFace(wireMaker.Wire());
    gp_Vec aPrismVec(0, 0, myHeight);
    TopoDS_Shape myBody = BRepPrimAPI_MakePrism(myFaceProfile, aPrismVec);



    // Building the Resulting Compound
    TopoDS_Compound aRes;
    BRep_Builder aBuilder;
    aBuilder.MakeCompound(aRes);
    aBuilder.Add(aRes, myBody);
//    aBuilder.Add(aRes, myThreading);

    return aRes;
}

int main(int argc, char **argv)
{

    TopoDS_Shape shape = MakeBottle(50, 70, 30);

    // Create document
    Handle(TDocStd_Document) aDoc;
    Handle(XCAFApp_Application) anApp = XCAFApp_Application::GetApplication();
    anApp->NewDocument("MDTV-XCAF", aDoc);

    // Create label and add our m_global_dims_
    Handle(XCAFDoc_ShapeTool) myShapeTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
    TDF_Label aLabel = myShapeTool->NewShape();
    myShapeTool->SetShape(aLabel, shape);

    // Write as STEP file
    STEPCAFControl_Writer *myWriter = new STEPCAFControl_Writer();
    myWriter->Perform(aDoc, "demo.stp");


}