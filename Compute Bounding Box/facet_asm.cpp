/* $RCSfile: facet_asm.cxx,v $
 * $Revision: 1.6 $ $Date: 2015/04/22 20:05:19 $
 * Auth: David Loffredo (loffredo@steptools.com)
 *
 * Copyright (c) 1991-2015 by STEP Tools Inc.
 * All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation is hereby granted, provided that this copyright
 * notice and license appear on all copies of the software.
 *
 * STEP TOOLS MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE
 * SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT. STEP TOOLS
 * SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A
 * RESULT OF USING, MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS
 * DERIVATIVES.
 */

// Sample program to facet  step assembly

#include <stp_schema.h>
#include <stix.h>
#include <stixmesh.h>
#include "context_utils.h"
#include "unit_utils.h"
#include <string>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <stdio.h>
#include <msclr\marshal_cppstd.h>
#include "make_box.h"

using namespace System::Runtime::InteropServices;
using namespace msclr::interop;

#define VERTEX_NORMAL_RENAMED_IN_LATEST_VERSIONS 3.14
#define FOUNDMAX(x) (std::cout << "Found " << #x << ": " << x << std::endl << "Enter new " << #x << " that is greater than or equal to found " << #x << ": ")
#define FOUNDMIN(x) (std::cout << "Found " << #x << ": " << x << std::endl << "Enter new " << #x << " that is less than or equal to found " << #x << ": ")

std::string prepend = "rawpiece_";

void user_modify_max(double &input);

void user_modify_min(double &input);

void user_modify_all_minmax_values(
	double &xmax,
	double &xmin,
	double &ymax,
	double &ymin,
	double &zmax,
	double &zmin);

void output_raw_piece(double &xmax, double &xmin, double &ymax, double &ymin, double &zmax, 
	double &zmin, StixUnit &angleUnit, StixUnit &lengthUnit, StixUnit &solidAngleUnit, std::string &fileName);

stp_shape_representation * create_empty_product_with_geometry(
    const StixUnit &au, const StixUnit &lu, const StixUnit &sau);

std::string run(char ** argv, double &xmax, double &xmin, double &ymax, double &ymin, double &zmax, double &zmin, std::string &lengthUnit)
{
	user_modify_all_minmax_values(xmax, xmin, ymax, ymin, zmax, zmin);
    ROSE.quiet(1);
    stplib_init();	// initialize merged cad library
    stixmesh_init();

	std::string outputFileName(argv[0]);
	outputFileName = prepend + outputFileName;

	StixUnit lu;
	if(lengthUnit == "in")
	{
		lu = stixunit_in;
	}
	else if(lengthUnit == "ft")
	{
		lu = stixunit_ft;
	}
	else if(lengthUnit == "mm")
	{
		lu = stixunit_mm;
	}
	else if(lengthUnit == "cm")
	{
		lu = stixunit_cm;
	}
	else if(lengthUnit == "m")
	{
		lu = stixunit_m;
	}
	else
	{
		std::cout << "Error: no matching length unit" << std::endl;
	}
	//just assuming that the angle unit is degrees
	StixUnit au = stixunit_deg;
	StixUnit sau = stixunit_steradian;

	output_raw_piece(xmax, xmin, ymax, ymin, zmax, zmin, lu, au, sau, outputFileName);

    system("pause");
    return outputFileName;
}

void output_raw_piece(double &xmax, double &xmin, double &ymax, double &ymin, double &zmax, 
	double &zmin, StixUnit &angleUnit, StixUnit &lengthUnit, StixUnit &solidAngleUnit, std::string &fileName)
{
    RoseDesign * d = new RoseDesign(fileName.c_str());
	ROSE.useDesign(d);
    stp_shape_representation* shape = create_empty_product_with_geometry(angleUnit, lengthUnit, solidAngleUnit);
    double x_width = (xmax - xmin);
    double y_width = (ymax - ymin);
    double z_width = (zmax - zmin);
    double x = (xmax + xmin) / 2;
    double y = (ymax + ymin) / 2;
    double z = zmax;
	std::cout << "From facet_asm.cpp" << std::endl;
    std::cout << std::endl << "x_width = " << x_width << std::endl << "y_width = "
	<< y_width << std::endl << "z_width = " << z_width << std::endl;
    char* color = "red";
    create_any_box(shape, x, y, z, x_width, y_width, z_width, color);
    d->save();
}

stp_shape_representation * create_empty_product_with_geometry(const StixUnit &au, const StixUnit &lu, const StixUnit &sau)
{
    //----------------------------------------
    // CREATE THE PRODUCT -
    // Create a product object to represent our part. 
    stp_product* pr = pnew stp_product();

    // Fill in the part number, name, and description. 
    pr->id("1234-K789");
    pr->name("widget");
    pr->description("a fictional product");

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
    pdf->description("first version of our widget");


    // The PDF needs a product definition.  This describes a the
    // version from a particular point of view in the life cycle, so
    // most APs only have one per PDF, but some APs use several.
    //
    stp_product_definition* pd = pnew stp_product_definition();
    pd->formation(pdf);
    pd->id("design");
    pd->description("example product_definition");

    // The context information is boilerplate information as above in
    // the product.  See the context_utils.cxx file for the specifics.
    // 
    pd->frame_of_reference(make_pdef_context());


    //----------------------------------------
    // Depending on the application protocol, the product, PDF, and PD
    // may also need some configuration management information, like a
    // design owner, approvals, etc.  For simplicity, we do not create
    // any of that here, but a real application would.


    //----------------------------------------
    // CREATE AND ATTACH A SHAPE REPRESENTATION
    //
    // The geometric description of product shape is attached to the
    // product_definition as a property.  The way properties work in
    // STEP is with a series of objects:
    //
    //   (owner of the property) 
    //      <- property_definition
    // 		<- property_definition_representation -> 
    //		      representation ->
    //			 representation_items  
    //
    // The property_definition identifies the property type while the
    // representation + rep items describe the property value.  The
    // property_definition_representation relates the property with
    // its value description.  In this way STEP can describe the value
    // of a particular property in several different ways if needed
    // (such as a brep version of geometry and a facetted version)
    // 
    // The geometric shape of a product uses some special subtypes of
    // the things above to make the shape property more distinctive in
    // the data set.
    //----------------------------------------


    // Create the shape representation.  This is the container that
    // holds geometry items.  Normally we would create some subtype of
    // shape rep like advanced_brep_shape_representation to indicate a
    // particular style of geometry/topology description.
    //
    stp_shape_representation * rep = pnew stp_shape_representation;

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




    //----------------------------------------
    // INITIALIZE THE SHAPE REPRESENTATION
    //
    // The STEP geometry definitions in ISO 10303-42 are unit-less, so
    // we must give the shape representation context information that
    // identifies the units, the number of dimensions (2 or 3), and
    // possibly some geometric uncertainty (how close two points can
    // be before they are considered the same).
    //
    // Refer to geometry_utils.cxx for specifics on the context.
    // Refer to unit_utils.cxx  for specifics on the units.
    //

    stp_representation_context * rep_ctx = stix_make_geometry_context(
	ROSE.design(),	// location to create the context
	"ID1", 		// some name for the context
	3, 		// number of dimensions, ie 3D
	lu, 	// length unit for geometry
	au,	// angle unit for geometry
	sau,	// solid angle unit
	1e-6		// global uncertainty (optional)
	);

    rep->name("");
    rep->context_of_items(rep_ctx);


    // Now we would add the geometry and topology items that define
    // the product shape.  For an example, create an axis placement
    // with a location but no direction vectors.
    //
    stp_cartesian_point* p1 = pnew stp_cartesian_point;
    p1->name("");
    p1->coordinates()->add(0.0);
    p1->coordinates()->add(0.0);
    p1->coordinates()->add(0.0);
    stp_axis2_placement_3d* ap3d = pnew stp_axis2_placement_3d(
	"orientation", p1, 0, 0
	);


    // Add it to the representation
    rep->items()->add(ap3d);

    // Done
    return rep;
}

//perhaps change these functions to
//modify them to be anything
void user_modify_max(double &input)
{
	double temp;
	std::cin >> temp;
	if(temp >= input)
	{
		input = temp;
	}
	else
	{
		std::cout << "Inputted Value is less than found value" << std::endl
			<< "No change will be made to found value" << std::endl;
	}
}

void user_modify_min(double &input)
{
	double temp;
	std::cin >> temp;
	if(temp <= input)
	{
		input = temp;
	}
	else
	{
		std::cout << "Inputted Value is greater than found value" << std::endl
			<< "No change will be made to found value" << std::endl;
	}
}

void user_modify_all_minmax_values(
	double &xmax,
	double &xmin,
	double &ymax,
	double &ymin,
	double &zmax,
	double &zmin)
{
	FOUNDMAX(xmax);
	user_modify_max(xmax);
	FOUNDMIN(xmin);
	user_modify_min(xmin);
	FOUNDMAX(ymax);
	user_modify_max(ymax);
	FOUNDMIN(ymin);
	user_modify_min(ymin);
	FOUNDMAX(zmax);
	user_modify_max(zmax);
	FOUNDMIN(zmin);
	user_modify_min(zmin);
}

public ref class BBoxer
{
	public:
		static System::String^ RunMain(System::String^ wpname, System::Double xmax, System::Double xmin, System::Double ymax,
			System::Double ymin, System::Double zmax, System::Double zmin, System::String^ unit)
		{
			char *foo[1];
			foo[0] = (char*)Marshal::StringToHGlobalAnsi(wpname).ToPointer();
			std::string s = marshal_as<std::string>(unit);
			std::string returnFileName = run(foo, xmax, xmin, ymax, ymin, zmax, zmin, s);
			System::String^ rfn = gcnew System::String(returnFileName.c_str());
			return rfn;
		}
};