using System;
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
            long count = f.GetPathAxisCount(path_id);
            for (long i = 0; i < count; ++i)
            {
                long cve_id = f.GetPathAxisNext(path_id, i);
                long count2 = f.GetPathPolylinePointCount(cve_id);
                for (int j = 0; j < count2; ++j)
                {
                    double ai, aj, ak;
                    f.GetPathPolylinePointNext(cve_id, j, out ai, out aj, out ak);
                    if (happenedOnce)
                    {
                        calc_min_max_vertices(ai, aj, ak);
                    }
                    else
                    {
                        xmin = xmax = ai;
                        ymin = ymax = aj;
                        zmin = zmax = ak;
                        happenedOnce = true;
                    }
                }
            }
        }

        static void decode_workingstep(Finder f, long exe_id)
        {
            bool isContact;
            long size = f.GetWorkingstepPathCount(exe_id);
            for (long i = 0; i < size; ++i)
            {
                long path_id = f.GetWorkingstepPathNext(exe_id, i, out isContact);
                decode_path(f, path_id);
            }
        }

        static void Main(string[] args)
        {
            string inputStepNCFileName = args[0];
            string toSend = inputStepNCFileName.Replace(".238", "");
            string finalFileName = "rawpieceaddedfromtoolpath_" + inputStepNCFileName;
            Finder find1 = new Finder();
            find1.Open238(inputStepNCFileName);
            long wp_id = find1.GetMainWorkplan();
            long size = find1.GetNestedExecutableCount(wp_id);
            for (int i = 0; i < size; ++i)
            {
                long exe_id = find1.GetNestedExecutableNext(wp_id,i);
                if(find1.IsWorkingstep(exe_id))
                {
                    decode_workingstep(find1, exe_id);
                }
            }
            AptStepMaker asm1 = new AptStepMaker();
            asm1.Open238(inputStepNCFileName);
            string rawMaterialName = BBoxer.RunMain(toSend, xmax, xmin, ymax, ymin, zmax, zmin, find1.GetWorkpieceUnits());
        }
    }
}
