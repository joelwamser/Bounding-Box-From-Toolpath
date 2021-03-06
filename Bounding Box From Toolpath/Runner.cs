﻿using System;
using System.Collections.Generic;
using System.Text;
using STEPNCLib;

namespace Bounding_Box_From_Toolpath
{
    class Runner
    {
        private static double xmax = 0.0;
        private static double xmin = 0.0;
        private static double ymax = 0.0;
        private static double ymin = 0.0;
        private static double zmax = 0.0;
        private static double zmin = 0.0;
        private static bool happenedOnce = false;
        private static string path_units = "unknown";

        static void calc_min_max_vertices(double x, double y, double z)
        {
            xmax = Math.Max(xmax, x);
            xmin = Math.Min(xmin, x);
            ymax = Math.Max(ymax, y);
            ymin = Math.Min(ymin, y);
            zmax = Math.Max(zmax, z);
            zmin = Math.Min(zmin, z);
        }

        static void decode_path(Finder f, long path_id)
        {
            path_units = f.GetPathUnits(path_id);
            //Console.WriteLine("Path units : {0}", path_units);
            //Console.ReadLine();
            long count = f.GetPathCurveCount(path_id);
            //Console.Write("Number of Path Curves: ");
            //Console.WriteLine(count);
            for (long i = 0; i < count; ++i)
            {
                //include else statement for GetPathArc and code for it as well
                bool isArc;
                long cve_id = f.GetPathCurveNext(path_id, i, out isArc);
                if(!isArc)
                {
                    long count2 = f.GetPathPolylinePointCount(cve_id);
                    for (int j = 0; j < count2; ++j)
                    {
                        double x, y, z;
                        f.GetPathPolylinePointNext(cve_id, j, out x, out y, out z);
                        if (happenedOnce)
                        {
                            calc_min_max_vertices(x, y, z);
                        }
                        else
                        {
                            xmin = xmax = x;
                            ymin = ymax = y;
                            zmin = zmax = z;
                            happenedOnce = true;
                        }
                    }
                }
                //NEED LOGIC TO HANDLE ARCS AND COMPUTE X,Y,Z
                //MAXES AND MINS FROM THE DATA
                 //(for now ignore arcs says Professor Hardwick)
                else
                {
                    /*double x;
                    double y;
                    double z;
                    double cx;
                    double cy;
                    double cz;
                    double radius;
                    bool ccw;
                    f.GetPathArc(cve_id, out x, out y, out z, out cx, out cy, out cz, out radius, out ccw);*/

                    //How do i figure out where the arc exists? I only have the end point.
                    //(Maybe use last point from polyline code as starting point?)
                    Console.WriteLine("isArc");
                    //This readline is here to halt the output so that
                    //I can see if one of these arcs ever exists in any
                    //of my test cases.
                    //Console.ReadLine();
                }
            }
        }

        static void decode_workingstep(Finder f, long exe_id)
        {
            bool isContact;
            long size = f.GetWorkingstepPathCount(exe_id);
            //Console.Write("Number of Workingstep Paths: ");
            //Console.WriteLine(size);
            for (long i = 0; i < size; ++i)
            {
                long path_id = f.GetWorkingstepPathNext(exe_id, i, out isContact);
                if(!f.GetPathRapid(path_id))
                    decode_path(f, path_id);
                //other wise it is a rapid path, so ignore it
            }
        }

        //this currently treats a selective as a workplan and executes all of the
        //executables that exist inside of it instead of just selecting one
        static void decode_selective(AptStepMaker a, Finder f, long exe_id, bool enFlags)
        {
            long size = f.GetSelectiveExecutableCount(exe_id);
            for (long i = 0; i < size; ++i)
            {
                long e_id = f.GetSelectiveExecutableNext(exe_id, i);
                Console.WriteLine("Executable Type from Selective: {0}", f.GetExecutableType(e_id));
                //if we care about whether or not executables are enabled
                //and if the executable has been disabled
                //then skip this executable
                if (enFlags && !a.GetExecutableIsEnabled(e_id))
                {
                    Console.WriteLine("Disabled!!");
                    //Console.ReadLine();
                    continue;
                }
                if (f.IsWorkingstep(e_id))
                {
                    decode_workingstep(f, e_id);
                }
                else if (f.IsWorkplan(e_id))
                {
                    decode_workplan(a, f, e_id, enFlags);
                }
                else if (f.IsSelective(e_id))
                {
                    decode_selective(a, f, e_id, enFlags);
                }
            }
        }

        static void decode_workplan(AptStepMaker a, Finder f, long wp_id, bool enFlags)
        {
            long size = f.GetNestedExecutableCount(wp_id);
            Console.Write("Number of nested executables: ");
            Console.WriteLine(size);
            for (int i = 0; i < size; ++i)
            {
                long exe_id = f.GetNestedExecutableNext(wp_id, i);
                Console.WriteLine("Executable Type: {0}", f.GetExecutableType(exe_id));
                //if we care about whether or not executables are enabled
                //and if the executable has been disabled
                //then skip this executable
                if (enFlags && !a.GetExecutableIsEnabled(exe_id))
                {
                    Console.WriteLine("Disabled!!");
                    //Console.ReadLine();
                    continue;
                }
                if (f.IsWorkingstep(exe_id) && f.GetProcessSpeed(exe_id) != 0)
                {
                    //double speed = f.GetProcessSpeed(exe_id);
                    //Console.WriteLine("Process Speed: {0}", f.GetProcessSpeed(path_id));
                    //Console.WriteLine(speed);
                    //Console.ReadLine();
                    decode_workingstep(f, exe_id);
                }
                else if (f.IsWorkplan(exe_id))
                {
                    decode_workplan(a, f, exe_id, enFlags);
                }
                else if (f.IsSelective(exe_id))
                {
                    decode_selective(a, f, exe_id, enFlags);
                }
                //else its an NC Function (unimportant right?)
            }
        }

        static bool prompt_user()
        {
            while (true)
            {
                string userInput = Console.ReadLine();
                if (userInput == "Yes" || userInput == "yes" || userInput == "y")
                {
                    return true;
                }
                else if (userInput == "No" || userInput == "no" || userInput == "n")
                {
                    return false;
                }
                else
                {
                    Console.WriteLine("Invalid entry. Try again");
                }
            }
        }

        static void Main(string[] args)
        {
            //int afjko = CylBoxer.RunMain();
            //Console.ReadLine();

            Console.WriteLine("Run program with Executable Enable Flags? Enter \"Yes\" or \"No\".");
            bool useEnableFlags = prompt_user();
            Console.WriteLine("Enter \"Yes\" to use Cylinder Stock. Enter \"No\" to use Rectangular Prism Stock.");
            bool useCylinerStock = prompt_user();
            string inputStepNCFileName = args[0];
            //this handles both .stpnc's and .238's
            string hold = inputStepNCFileName.Replace(".238", "");
            string toSend = hold.Replace(".stpnc", "");
            string finalFileName = "rawpieceaddedfromtoolpath_" + inputStepNCFileName;
            AptStepMaker asm1 = new AptStepMaker();
            asm1.Open238(inputStepNCFileName);
            Finder find1 = new Finder();
            find1.Open238(inputStepNCFileName);
            long wp_id = find1.GetMainWorkplan();
            decode_workplan(asm1, find1, wp_id, useEnableFlags);
            Console.WriteLine("Path units: {0}", path_units);
            Console.WriteLine("Xmax: {0} ", xmax);
            Console.WriteLine("Xmin: {0} ", xmin);
            Console.WriteLine("Ymax: {0} ", ymax);
            Console.WriteLine("Ymin: {0} ", ymax);
            Console.WriteLine("Zmax: {0} ", zmax);
            Console.WriteLine("Zmin: {0} ", zmin);
            string rawMaterialName = BBoxer.RunMain(toSend, xmax, xmin, ymax, ymin, zmax, zmin, path_units, useCylinerStock);
            asm1.Rawpiece(rawMaterialName);
            asm1.SaveAsP21(finalFileName);
        }
    }
}
