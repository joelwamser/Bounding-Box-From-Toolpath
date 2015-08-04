#include <rose.h>
#include <stp_schema.h>
#include <stix.h>
#include <stixmesh.h>
#include "cylinder.h"
#include "geometry_utils.h"
#include "context_utils.h"
#include "unit_utils.h"

int run()
{
	/* Force optimizing linkers to bring in all C++ classes */
	stplib_init();

	/* Create a RoseDesign to hold the instance data */
	const char * output_name = "cylinder";
	RoseDesign * design = new RoseDesign(output_name);

	/* Give the design some Part 21 header information */
	design->initialize_header();
	design->header_name()->originating_system("Cylinder Creation");
	design->header_description()->description()->
		add("Empty Shape Representation Sample File");

	ROSE.useDesign(design);

	StixUnit au = stixunit_deg;
	StixUnit lu = stixunit_mm;
	StixUnit sau = stixunit_steradian;

	stp_advanced_brep_shape_representation * cylin = create_empty_product_with_geometry_cylinder(
		au, lu, sau);

	const char* c = "red";

	create_any_cylinder(cylin, 5, 10, 2.5, 2.5, 5, c);

	design->save();
	return 0;
}

stp_advanced_brep_shape_representation * create_empty_product_with_geometry_cylinder(
	StixUnit &au, StixUnit &lu, StixUnit &sau)
{
	//----------------------------------------
	// CREATE THE PRODUCT -
	// Create a product object to represent our part. 
	stp_product* pr = pnew stp_product();

	// Fill in the part number, name, and description. 
	pr->id("1234-K789");
	pr->name("Cylinder");
	pr->description("the start of something new");

	// The context information is boilerplate information that is the
	// same for all files defined by a particular AP, although it does
	// change a bit from AP to AP.  See the context_utils.cxx file for
	// the specifics.
	//
	pr->frame_of_reference()->add(make_product_context());


	// The product needs a product definition formation to describe
	// the version with an id and description
	// 
	stp_product_definition_formation* pdf =
		pnew stp_product_definition_formation;

	pdf->of_product(pr);
	pdf->id("1.0");
	pdf->description("first version of cylinder creator");


	// The PDF needs a product definition.  This describes the
	// version from a particular point of view in the life cycle, so
	// most APs only have one per PDF, but some APs use several.

	stp_product_definition* pd = pnew stp_product_definition();
	pd->formation(pdf);
	pd->id("design");
	pd->description("Great Cylinder");

	// The context information is boilerplate information as above in
	// the product.  See the context_utils.cxx file for the specifics.
	// 
	pd->frame_of_reference(make_pdef_context());

	stp_advanced_brep_shape_representation * rep = pnew stp_advanced_brep_shape_representation;

	// Give the product a shape property.  Product_definition_shape is
	// a subtype of property_definition used for shape properties.  It 
	// refers to the product through the product_definition.
	//
	stp_product_definition_shape * pds =
		pnew stp_product_definition_shape;

	pds->name("");
	pds->definition(pnew stp_characterized_definition);
	pds->definition()->_characterized_product_definition
		(pnew stp_characterized_product_definition);
	pds->definition()->_characterized_product_definition()->
		_product_definition(pd);


	// Attach the shape representation to the property.  The
	// shape_definition_representation subtype is used for shape
	// properties and the property_definition_representation supertype
	// is used for other types of properties.
	//
	stp_shape_definition_representation * sdr =
		pnew stp_shape_definition_representation;

	sdr->definition(pnew stp_represented_definition);
	sdr->definition()->_property_definition(pds);
	sdr->used_representation(rep);

	stp_representation_context * rep_ctx = stix_make_geometry_context(
		ROSE.design(),	// location to create the context
		"ID1", 		// some name for the context
		3, 		// number of dimensions, ie 3D
		lu, 	// length unit for geometry
		au,	// angle unit for geometry
		sau,	// solid angle unit
		1e-6
		);

	rep->name("");
	rep->context_of_items(rep_ctx);

	return rep;
}

//the x, y, and z points here are the middle of the top face of the cylinder
void create_any_cylinder(stp_advanced_brep_shape_representation * advsol, const double &diam, 
	const double &hei, const double &x, const double &y, const double &z, const char *color){
	
	//Variable to create radius entity for various entities used
	double temp = diam / 2.0;

	//Final axis placements and cartesian points/vectors
	stp_cartesian_point* p1 = pnew stp_cartesian_point;
	p1->name("");
	p1->coordinates()->add(x);
	p1->coordinates()->add(y);
	p1->coordinates()->add(z-hei);

	stp_direction * dir1 = pnew stp_direction;
	dir1->direction_ratios()->add(0);
	dir1->direction_ratios()->add(0);
	dir1->direction_ratios()->add(1);

	stp_direction * dir2 = pnew stp_direction;
	dir2->direction_ratios()->add(1);
	dir2->direction_ratios()->add(0);
	dir2->direction_ratios()->add(0);

	stp_axis2_placement_3d * a2p32 = pnew stp_axis2_placement_3d("", p1, dir1, dir2);

	stp_axis2_placement * a2p4 = pnew stp_axis2_placement;
	rose_put_nested_object(a2p4, a2p32);

	stp_cylindrical_surface * cs1 = pnew stp_cylindrical_surface;
	cs1->position(a2p32);
	cs1->radius(temp);
	
	stp_cartesian_point* p5 = pnew stp_cartesian_point;
	p5->name("");
	p5->coordinates()->add(x);
	p5->coordinates()->add(y);
	p5->coordinates()->add(z);
	
	stp_vector * vec1 = pnew stp_vector;
	vec1->orientation(dir1);
	vec1->magnitude(1);
	
	stp_axis2_placement_3d* ap3d = pnew stp_axis2_placement_3d("", p5, dir1, dir2);

	stp_axis2_placement * a2p1 = pnew stp_axis2_placement;
	rose_put_nested_object(a2p1, ap3d);

	//Pcurve and surface geometry definition
	stp_cartesian_point* p3 = pnew stp_cartesian_point;
	p3->name("");
	p3->coordinates()->add(x+temp);
	p3->coordinates()->add(y);
	p3->coordinates()->add(z);

	//needed
	stp_circle * circ1 = pnew stp_circle;
	circ1->position(a2p1);
	circ1->radius(temp);

	//needed
	stp_cartesian_point* p4 = pnew stp_cartesian_point;
	p4->name("");
	p4->coordinates()->add(x+temp);
	p4->coordinates()->add(y);
	p4->coordinates()->add(z-hei);

	stp_line * l1 = pnew stp_line;
	l1->pnt(p4);
	l1->dir(vec1);

	//needed
	stp_circle * circ2 = pnew stp_circle;
	circ2->position(a2p4);
	circ2->radius(temp);

	//Vertex Point and Surface Curve definition
	stp_vertex_point * vp1 = pnew stp_vertex_point;
	vp1->vertex_geometry(p3);

	//needed
	stp_vertex_point * vp2 = pnew stp_vertex_point;
	vp2->vertex_geometry(p4);	
	
	//Edge Curve definition
	stp_edge_curve * ec1 = pnew stp_edge_curve;
	ec1->edge_start(vp1);
	ec1->edge_end(vp1);
	ec1->edge_geometry(circ1);
	ec1->same_sense(1);

	stp_edge_curve * ec2 = pnew stp_edge_curve;
	ec2->edge_start(vp2);
	ec2->edge_end(vp1);
	ec2->edge_geometry(l1);
	ec2->same_sense(1);

	stp_edge_curve * ec3 = pnew stp_edge_curve;
	ec3->edge_start(vp2);
	ec3->edge_end(vp2);
	ec3->edge_geometry(circ2);
	ec3->same_sense(1);


	//Oriented Edge Definitions
	stp_oriented_edge * oe1 = pnew stp_oriented_edge;
	oe1->edge_element(ec1);
	oe1->orientation(0);

	stp_oriented_edge * oe2 = pnew stp_oriented_edge;
	oe2->edge_element(ec2);
	oe2->orientation(0);

	stp_oriented_edge * oe3 = pnew stp_oriented_edge;
	oe3->edge_element(ec3);
	oe3->orientation(1);

	stp_oriented_edge * oe4 = pnew stp_oriented_edge;
	oe4->edge_element(ec2);
	oe4->orientation(1);

	stp_oriented_edge * oe5 = pnew stp_oriented_edge;
	oe5->edge_element(ec1);
	oe5->orientation(1);

	stp_oriented_edge * oe6 = pnew stp_oriented_edge;
	oe6->edge_element(ec3);
	oe6->orientation(0);

	//Edge Loop definition and Axis2 Placements
	stp_edge_loop * el1 = pnew stp_edge_loop;
	el1->edge_list()->add(oe1);
	el1->edge_list()->add(oe2);
	el1->edge_list()->add(oe3);
	el1->edge_list()->add(oe4);
	
	stp_edge_loop * el2 = pnew stp_edge_loop;
	el2->edge_list()->add(oe5);

	stp_cartesian_point* p2 = pnew stp_cartesian_point;
	p2->name("");
	p2->coordinates()->add(x);
	p2->coordinates()->add(y);
	p2->coordinates()->add(z-hei);

	stp_axis2_placement_3d * a2p33 = pnew stp_axis2_placement_3d("", p2, dir1, dir2);

	stp_edge_loop * el3 = pnew stp_edge_loop;
	el3->edge_list()->add(oe6);


	//Face bounds and surface declarations
	stp_face_bound * b1 = pnew stp_face_bound;
	b1->bound(el1);
	b1->orientation(1);

	stp_face_bound * b2 = pnew stp_face_bound;
	b2->bound(el2);
	b2->orientation(1);

	stp_plane * plane1 = pnew stp_plane;
	plane1->position(a2p33);

	stp_face_bound * b3 = pnew stp_face_bound;
	b3->bound(el3);
	b3->orientation(1);

	stp_plane * plane2 = pnew stp_plane;
	plane2->position(a2p32);
	
	//Advanced Faces declarations
	stp_advanced_face * f1 = pnew stp_advanced_face;
	f1->bounds()->add(b1);
	f1->face_geometry(cs1);
	f1->same_sense(1);

	stp_advanced_face * f2 = pnew stp_advanced_face;
	f2->bounds()->add(b2);
	f2->face_geometry(plane1);
	f2->same_sense(1);

	stp_advanced_face * f3 = pnew stp_advanced_face;
	f3->bounds()->add(b3);
	f3->face_geometry(plane2);
	f3->same_sense(0);

	//Closed Shell Declaration
	stp_closed_shell * sh = pnew stp_closed_shell;
	sh->cfs_faces()->add(f1);
	sh->cfs_faces()->add(f2);
	sh->cfs_faces()->add(f3);
	
	//Manifold solid brep and axis2_placement_3d 
	stp_manifold_solid_brep * brep = pnew stp_manifold_solid_brep;
	brep->outer(sh);
	advsol->items()->add(brep);

	if (color)
	{
		color_face (f1, color);
		color_face (f2, color);
		color_face (f3, color);
	}

	return;
}

public ref class CylBoxer
{
	public:
		static int RunMain()
		{
			return run();
		}
};