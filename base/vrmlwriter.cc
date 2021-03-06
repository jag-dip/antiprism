/*
   Copyright (c) 2003-2008, Adrian Rossiter

   Antiprism - http://www.antiprism.com

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

      The above copyright notice and this permission notice shall be included
      in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

/* \file VrmlWriter.cc
   \brief write a VRML file
*/

#include <string>
#include <vector>

#include "symmetry.h"
#include "vrmlwriter.h"

using std::string;
using std::vector;

namespace anti {

// functions -----------------------------------------------

string vrml_vec(double x, double y, double z, int sig_digits)
{
  char buf[256];
  if (sig_digits > 0)
    snprintf(buf, 256, "%.*g %.*g %.*g", sig_digits, x, sig_digits, y,
             sig_digits, z);
  else
    snprintf(buf, 256, "%.*f %.*f %.*f", -sig_digits, x, -sig_digits, y,
             -sig_digits, z);
  return buf;
}

string vrml_col(const Color &col)
{
  Vec4d cv = col.get_vec4d();
  char buf[256];
  snprintf(buf, 256, "%.4f %.4f %.4f", cv[0], cv[1], cv[2]);
  // if(col.get_trans())
  //   snprintf(buf, 256, "<%g, %g, %g, %g>", cv[0], cv[1], cv[2], cv[3]);
  // else
  //   snprintf(buf, 256, "<%g, %g, %g>", cv[0], cv[1], cv[2]);
  return buf;
}

// --------------------------------------------------------------
// VrmlWriter

void VrmlWriter::write(FILE *ofile, Scene &scen, int sig_digits)
{
  header(ofile);
  scene_header(ofile, scen);
  cameras(ofile, scen);
  geometry_objects(ofile, scen, sig_digits);
}

void VrmlWriter::header(FILE *ofile)
{
  fprintf(ofile, "#VRML V2.0 utf8\n"
                 "\n"
                 "NavigationInfo {\n"
                 "   type [\"EXAMINE\", \"ANY\"]\n"
                 "}\n"
                 "\n");
}

void VrmlWriter::scene_header(FILE *ofile, Scene &scen)
{
  fprintf(ofile,
          "Background { skyColor [ %s ] }\n"
          "\n"
          "DirectionalLight { intensity 0 ambientIntensity 0.5}\n"
          "\n",
          vrml_col(scen.get_bg_col()).c_str());
}

void VrmlWriter::cameras(FILE *ofile, Scene &scen)
{
  fprintf(ofile, "Group {\n"
                 "   children [\n");

  for (int i = scen.get_cameras().size() - 1; i >= 0; --i) {
    const Camera &cam = scen.get_cameras()[i];
    double offset = cam.get_distance() * cam.get_persp();
    Trans3d inv_rot = cam.get_rotation().inverse();
    Vec3d cam_pos = inv_rot * Vec3d(0, 0, offset);

    Isometry ax_ang(inv_rot);
    Vec3d axis(0, 1, 0);
    if (ax_ang.get_axis().is_set())
      axis = ax_ang.get_axis();

    fprintf(ofile,
            "      DEF %s Viewpoint {\n"
            "         position %s\n"
            "         orientation %s %g\n"
            "         fieldOfView %g\n"
            "         description \"%s\"\n"
            "      }\n",
            scen.get_camera_name(i).c_str(), vrml_vec(cam_pos).c_str(),
            vrml_vec(axis).c_str(), ax_ang.get_ang(), 0.78 / cam.get_persp(),
            scen.get_camera_name(i).c_str());
  }

  fprintf(ofile, "   ]\n"
                 "}\n"
                 "\n");
}

void VrmlWriter::geometry_objects(FILE *ofile, const Scene &scen,
                                  int sig_digits)
{
  vector<SceneGeometry>::const_iterator geo;
  for (geo = scen.get_geoms().begin(); geo != scen.get_geoms().end(); ++geo) {
    fprintf(ofile, "# Start of geometry %s\n\n", geo->get_name().c_str());
    vector<GeometryDisplay *>::const_iterator disp;
    for (disp = geo->get_disps().begin(); disp != geo->get_disps().end();
         ++disp)
      (*disp)->vrml_geom(ofile, scen, sig_digits);
    if (geo->get_label())
      geo->get_label()->vrml_geom(ofile, scen, sig_digits);
    if (geo->get_sym())
      geo->get_sym()->vrml_geom(ofile, scen, sig_digits);
  }
}

} // namespace anti
