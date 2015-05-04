/*
   Copyright (c) 2014, Roger Kaufman and Adrian Rossiter

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

/*
   Name: symmetro.cc
   Description: Make symmetrohedra
   Project: Antiprism - http://www.antiprism.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <float.h>

#include <string>
#include <vector>
#include <algorithm>

#include "../base/antiprism.h"

using std::string;
using std::vector;
using std::fill;
using std::swap;


class symmetro_opts: public prog_opts {
   public:
      char sym;
      int sym_id_no;
      int p;
      int q;
      vector<int> multipliers;
      vector<int> d;
      int dihedral_n;
      vector<int> rotation_axis;
      double rotation_as_increment;
      double rotation;
      bool add_pi;
      vector<int> ratio_direction;
      double ratio;
      int convex_hull;
      bool verbose;
      int mode;
      
      vector<int> col_axis_idx;
      char face_coloring_method;
      int face_opacity;
      col_val vert_col;
      col_val edge_col;
      color_map_multi map;
      
      string ofile;

      symmetro_opts(): prog_opts("symmetro"),
                       sym('\0'),
                       sym_id_no(1),
                       p(0),
                       q(0),
                       dihedral_n(0),
                       rotation_as_increment(0.0),
                       rotation(0.0),
                       add_pi(false),
                       ratio(0.0),
                       convex_hull(0),
                       verbose(false),
                       mode(0),
                       face_coloring_method('a'),
                       face_opacity(255),
                       vert_col(col_val(255,215,0)),  // gold
                       edge_col(col_val(211,211,211)) // lightgrey
                       {}
      
      void process_command_line(int argc, char **argv);
      void usage();
};

void symmetro_opts::usage()
{
   fprintf(stdout,
"\n"
"Usage: %s [options]\n"
"\n"
"Symmetrohedra and Twisters are created by placing equilateral polygons centered\n"
"on the symmetry axes of Icosahedral, Octahedral, Tetrahedral, or Dihedral\n"
"symmetry. The sides of the polygons will be a multiple number of the axis\n"
"reflection number.\n"
"\n"
"It is possible to generate models such that the polygons intersect. If a\n"
" collision is detected, convex hull will be suppressed\n"
"\n"
"options -k, -t, and -d cannot be used together, but one needs to be specified\n" 
"\n"
"Options\n"
"%s"
"  -k <s,l,m,n,a> Kaplan-Hart notation. Generate Symmetrohedra based on a study\n"
"            by Craig S. Kaplan and George W. Hart (http://www.georgehart.com).\n"
"            Project url: http://www.cgl.uwaterloo.ca/~csk/projects/symmetrohedra\n"
"            s: symmetry type of Symmetrohedra. sets {p,q,2}\n"
"               I-icosahedral {5,3,2} O-octahedral {4,3,2} T-tetrahedral {3,3,2}\n"
"            l,m,n: multipliers for axis polygons. Separated by commas, one\n"
"               multiplier must be * or 0, the other two are positive integers\n"
"            a: face rotation type: vertex=1, edge=0  (default: 1)\n"
"            example: -k i,2,*,4,e\n"
"  -t <s[p,q],i,m1,m2> Twister notation. Generate twister models.\n"
"            s: symmetry. I-icosahedral, O-octahedral, T-tetrahedral, D-dihedral\n"
"            p,q: rotational order of each of the two axes\n"
"            i: (default: 1): integer to select between non-equivalent pairs of\n"
"               axes having the same symmetry group and rotational orders\n"
"            m1,m2: an integer multiplier for each axis. i.e. m1*p and m2*q\n"
"               also can be entered as m1/d, m2/d fractional values\n"
"               e.g. T[2,3],3,2  I[5,2]2,5/2,6  D7[7,3],1,2, D11[2,2]5,2,2\n"
"                  Axis pairs are from the following\n"
"                  T: [3, 3], [3, 2], [2, 2]\n"
"                  O: [4, 4], [4, 3], [4, 2]x2, [3, 3], [3, 2]x2, [2, 2]x2\n"
"                  I: [5, 5], [5, 3]x2, [5, 2]x3, [3, 3]x2, [3, 2]x4, [2, 2]x4\n"
"                  DN:[N, q] q>1, [2,2]x(n/2 rounded down)\n"
"  -d <n/d>  Twisters with S symmetry. n and d must be odd. denominator optional\n"
"  -a <a,n>  a in degrees of rotation given to polygon applied to optional axis n\n"
"               if n not given, implies first axis encountered\n"
"               radians may be entered as 'rad(a)'\n"
"  -r <r,n>  ratio r of axis n polygon. if n is not specified, implies first axis\n"
"               encountered e.g. 0.5,1 (default: calculated for unit edge length)\n"
"  -C <mode> convex hull. polygons=1, suppress=2, force=3, normal=4  (default: 4)\n"
"  -v        verbose output\n"
"  -o <file> write output to file (default: write to standard output)\n"
"\nColoring Options (run 'off_util -H color' for help on color formats)\n"
"  -V <col>  vertex color (default: gold)\n"
"  -E <col>  edge color   (default: lightgray)\n"
"  -f <mthd> mthd is face coloring method using color in map (default: a)\n"
"               key word: none - sets no color\n"
"               a - color by axis number\n"
"               n - color by number of sides\n"
"  -T <tran> face transparency. valid range from 0 (invisible) to 255 (opaque)\n"
"  -m <maps> color maps for faces to be tried in turn (default: m1)\n"
"               keyword m1: red,darkorange1,yellow,saddlebrown\n"
"                  note: position 4 color is for faces added by convex hull\n"
"               keyword m2: approximating colors in the symmetrohedra pdf file\n"
"\n"
"\n", prog_name(), help_ver_text);
}

// from StackOverflow
// remove blanks from input char*
char* deblank(char *input)                                         
{
   int i,j;
   char *output=input;

   for ( i = 0, j = 0; i<(int)strlen(input); i++, j++ ) {       
      if ( input[i] != ' ' )
         output[j] = input[i];
      else
         j--;
   }
   output[j] = '\0';
   
   return output;
}

void symmetro_opts::process_command_line(int argc, char **argv)
{
   opterr = 0;
   char c;
   char errmsg[MSG_SZ];
   
   string id;
   string map_file;
   
   vector<int> n;
   
   handle_long_opts(argc, argv);

   while( (c = getopt(argc, argv, ":hk:t:m:d:a:r:C:vf:V:E:T:o:")) != -1 ) {
      if(common_opts(c, optopt))
         continue;

      switch(c) {
          // Kaplan-Hart notation
         case 'k': {
            if ( mode )
               error("-k and -t cannot be used together", c);
            mode = 1;
           
            char parse_key1[] = ",";
            
            // memory pointers for strtok_r
            char *tok_ptr1;
           
            char *opts = deblank(optarg);
            
            vector<string> tokens;               
            char *ptok1 = strtok_r(opts,parse_key1,&tok_ptr1);
            while( ptok1 != NULL ) {
               tokens.push_back(ptok1);
               ptok1 = strtok_r(NULL,parse_key1,&tok_ptr1);
            }
            
            int sz = (int)tokens.size();
            if ( sz < 4 || sz > 5 )
               error("expecting 4 or 5 parameters for Kaplan-Hart notation", c);
            else
            if ( sz == 4 ) {
               tokens.push_back("v");
               sz++;
            }
            
            string mult;
            int num_multipliers = 0;
            for( int i=0; i<sz; i++ ) {
               if ( i == 0 ) {
                  sym = toupper(tokens[i][0]);
                  if ( !strchr("TOI", sym) )
                     error(msg_str("invalid symmetry character '%c'", sym), c);
               }
               else
               if ( i == 1 || i == 2 ) {
                  string tok = tokens[i];
                  if ( tok.length() == 1 && tok[0] == '*' )
                     tok = "0";
                  mult += tok + ",";
               }
               else
               if ( i == 3 ) {
                  string tok = tokens[i];
                  if ( tok.length() == 1 && tok[0] == '*' )
                     tok = "0";
                  mult += tok;
                  
                  // string to char * (not const) from StackOverflow
                  char *writable = new char[mult.size() + 1];
                  copy(mult.begin(), mult.end(), writable);
                  writable[mult.size()] = '\0';
                  
                  if(!read_int_list(writable, multipliers, errmsg, true, 3))
                     error(errmsg, c);
                  delete[] writable;
                  
                  // might not be able to happen
                  if( (int)multipliers.size() != 3 )
                     error("3 multipliers must be specified", c);
                     
                  for( int i=0; i<(int)multipliers.size(); i++ ) {
                     if ( multipliers[i]>0 )
                        num_multipliers++;
                  }
                  if ( num_multipliers == 0 )
                     error("at least one axis multiplier must be specified",c);
                  else
                  if ( num_multipliers == 3 )
                     error("at least one axis multiplier must be * or zero",c);
                     
                  if( multipliers[2] == 1 )
                     warning("model will contain digons");
                  
                  int orders[3] = { 0,3,2 };
                  orders[0] = ( sym == 'T' ) ? 3 : ( ( sym == 'O' ) ? 4 : 5 );

                  if ( num_multipliers == 1 ) {
                     if ( multipliers[0] ) {
                        p = orders[0];
                        q = orders[0];
                        if ( sym == 'T' )
                           rotation += 120.0;
                     }
                     else
                     if ( multipliers[1] ) {
                        p = orders[1];
                        q = orders[1];
                        if ( sym == 'T' )
                           rotation += 120.0;
                     }
                     else
                     if ( multipliers[2] ) {
                        p = orders[2];
                        q = orders[2];
                     }
                  }
                  else
                  if ( num_multipliers == 2 ) {
                     if ( multipliers[0] && multipliers[1] ) {
                        p = orders[0];
                        q = orders[1];
                     }
                     else
                     if ( multipliers[0] && multipliers[2] ) {
                        p = orders[0];
                        q = orders[2];
                     }
                     else
                     if ( multipliers[1] && multipliers[2] ) {
                        p = orders[1];
                        q = orders[2];
                     }
                  }
               }
               else
               if ( i == 4 ) {
                  // in the paper, edge connection is shown as 'e', vertex connection is shown a '1'
                  id = get_arg_id(tokens[i].c_str(), "edge=0|vertex=1", argmatch_add_id_maps, errmsg);
                  if(id=="")
                     error(errmsg);
                  if ( id == "0" )
                     rotation_as_increment = rad2deg(1.0);
                  else
                  if ( id == "1" )
                     rotation_as_increment = rad2deg(0.0);
               }
            }
            
            // for octahedral and icosahedral, axis2 alone
            if ( num_multipliers == 1 && multipliers[2] ) {
               if ( id == "1" ) { // vertex connected
                  // rotate to coincident faces
                  if ( sym == 'O' ) {
                     rotation += rad2deg(acos(1.0/3.0)/2.0); // 35.26438968275465431577 degrees
                  }
                  else
                  if ( sym == 'I' ) {
                     rotation += rad2deg(acos(2.0/sqrt(5.0))/2.0); // 13.28252558853899467604 degrees
                     if ( !is_even( multipliers[2] ) )
                        rotation += 90.0/(multipliers[2]*2.0);
                  }
               }
            }
            
            break;
         }
         
         // twister notation
         case 't': {
            if ( mode )
               error("-k and -t cannot be used together", c);
            mode = 2;
                        
            char parse_key1[] = ",[]";
            
            // memory pointer for strtok_r
            char *tok_ptr1;
            
            char *opts = deblank(optarg);
            bool id_no_given = ( strstr(opts,"],") ) ? false : true;
            
            vector<string> tokens;               
            char *ptok1 = strtok_r(opts,parse_key1,&tok_ptr1);
            while( ptok1 != NULL ) {
               tokens.push_back(ptok1);
               ptok1 = strtok_r(NULL,parse_key1,&tok_ptr1);
            }
            
            if ( !id_no_given ) {
               vector<string>::iterator it;
               it = tokens.begin();
               tokens.insert(it+3,"1");
            }
            
            int sz = (int)tokens.size();
            if ( sz != 6 )
               error("incorrect format for Twister notation", c);
            
            int mult_tok_num = 0;
            for( int i=0; i<sz; i++ ) {
               if ( i == 0 ) {
                  sym = toupper(tokens[i][0]);
                  if ( !strchr("TOID", sym) )
                     error(msg_str("invalid symmetry character '%c'", sym), c);
                     
                  // dihedral
                  if ( sym == 'D') {
                     if ( tokens[i].length() < 2 )
                        error("No N found after D symmetry specifier", c);  
                     if (!read_int(tokens[i].c_str()+1, &dihedral_n, errmsg))
                        error(errmsg, "dihedral symmetry N");
                  }
               }
               else
               if ( i == 1 ) {
                  if(!read_int(tokens[i].c_str(), &p, errmsg))
                     error(errmsg, "axis 1");
                  if ( p < 2 )
                     error("axis 1 rotational order number be greater than 1", c);
                  
                  if ( sym == 'D' && p != dihedral_n && p != 2 )
                     error(msg_str("when symmetry is D, axis 1 rotational order must equal 2 or N (%d)", dihedral_n), c);
               }
               else
               if ( i == 2 ) {
                  if(!read_int(tokens[i].c_str(), &q, errmsg))
                     error(errmsg, "axis 2");
                  if ( q < 2 )
                     error("axis 2 rotational order number be greater than 1", c);
                     
                  if ( sym == 'D' && q != 2 )
                     error("when symmetry is D, axis 2 rotational order must equal 2", c);
               }
               else
               if ( i == 3 ) {
                  if(!read_int(tokens[i].c_str(), &sym_id_no, errmsg))
                     error(errmsg, "symmetry id number");
                  if (sym_id_no<=0)
                     error("symmetry id number must be positive", c);
               }
               else
               if ( i == 4 || i == 5 ) {
                  if ( !strchr(tokens[i].c_str(),'/') ) {
                     int mult;
                     if(!read_int(tokens[i].c_str(), &mult, errmsg))
                        error(errmsg, "multiplier");
                     if (mult<=0)
                        error("multiplier must be positive", c);
                     multipliers.push_back(mult);
                  }
                  else {
                     mult_tok_num = i;
                     
                     char parse_key2[] = "/";
            
                     // memory pointer for strtok_r
                     char *tok_ptr2;
            
                     int n_part;
                     int d_part;
                     
                     // string to char * (not const) from StackOverflow
                     char *writable = new char[tokens[i].size() + 1];
                     copy(tokens[i].begin(), tokens[i].end(), writable);
                     writable[tokens[i].size()] = '\0';
                     
                     char *ptok2 = strtok_r(writable,parse_key2,&tok_ptr2);
                     int count2 = 0;
                     while( ptok2 != NULL ) {
                        if ( count2 == 0 ) {
                           if(!read_int(ptok2, &n_part, errmsg))
                              error(errmsg, "n/d (n part)");
                              
                           if (n_part<=0)
                              error("n of n/d must be positive", c);
                           n.push_back(n_part);
                        }
                        else
                        if ( count2 == 1 ) {
                           if(!read_int(ptok2, &d_part, errmsg))
                              error(errmsg, "n/d (d part)");
                              
                           if (d_part<=0)
                              error("d of n/d must be positive", c);
                           d.push_back(d_part);
                        }
                        
                        ptok2 = strtok_r(NULL,parse_key2,&tok_ptr2);
                        count2++;
                     }
                     delete[] writable;
                     
                     // if there is no denominator then it is 1
                     if ( (int)n.size() > (int)d.size() )
                        d.push_back(1);
                  }
               }
            }
            
            if ( (int)n.size() == 1 ) {
               if ( (int)multipliers.size() < 1 )
                  error("in multipliers specification",c);
                  
               // goes on end
               if ( mult_tok_num == 4 ) {
                  n.push_back(multipliers[0]);
                  d.push_back(1);
               }
               // goes at beginning
               else {
                  vector<int>::iterator it;
                  it = n.begin();
                  n.insert(it,multipliers[0]);
                  it = d.begin();
                  d.insert(it,1);
               }
               multipliers.clear();
               
               //warning(msg_str("axis %d multiplier interpreted as %d/1",( mult_tok_num == 4 ? 2 : 1 ), multipliers[0]), 'c');
            }
            
            if ( sym == 'D' ) {
               if ( p != 2 && ( ( n.size() && n[0] != 1 ) || ( multipliers.size() && multipliers[0] != 1 ) ) )
                  error("when symmetry is D, multiplier 1 must equal 1", c);
            }
         
            break;
         }
         
         // twister notation
         case 'd': {
            if ( mode )
               error("-k, -t, -d cannot be used together", c);
            mode = 3;
            
            char parse_key1[] = ",";
            char parse_key2[] = "/";
            
            // memory pointers for strtok_r
            char *tok_ptr1;
            char *tok_ptr2;
               
            char *ptok1 = strtok_r(optarg,parse_key1,&tok_ptr1);
            while( ptok1 != NULL ) {
               int n_part;
               int d_part;
               
               char *ptok2 = strtok_r(ptok1,parse_key2,&tok_ptr2);
               int count2 = 0;
               while( ptok2 != NULL ) {
                  if ( count2 == 0 ) {
                     if(!read_int(ptok2, &n_part, errmsg))
                        error(errmsg, "n/d (n part)");
                        
                     if (n_part<0)
                        error("n of n/d must be non-negative", c);
                     n.push_back(n_part);
                  }
                  else
                  if ( count2 == 1 ) {
                     if(!read_int(ptok2, &d_part, errmsg))
                        error(errmsg, "n/d (d part)");
                        
                     if (d_part<=0)
                        error("d of n/d must be positive", c);
                     d.push_back(d_part);
                  }
                  
                  ptok2 = strtok_r(NULL,parse_key2,&tok_ptr2);
                  count2++;
               }
               
               // if there is no denominator then it is 1
               if ( (int)n.size() > (int)d.size() )
                  d.push_back(1);               
                
               ptok1 = strtok_r(NULL,parse_key1,&tok_ptr1);
               count2++;
            }
            
            if ( (int)n.size() > 1 )
               error("only one n/d should be specified", c);
            else
            if ( (int)n.size() == 1 ) {
               n.push_back(n[0]);
               d.push_back(d[0]);               
            }

            if ( is_even(n[0]) )
               error("fractional numerator n must be odd", c);
               
            if ( (double)n[0]/(double)d[0] < 1.5 )
               error("polygon: the polygon fraction cannot be less than 3/2 (base rhombic tiling is not constructible)", c);
               
            if ( is_even(d[0]) )
               error("fraction denominator d should be odd", c);
               
            sym = 'S';
            p = n[0];
            q = n[1];
            dihedral_n = n[0]; 
                             
            break;
         }
          
         // rotation  
         case 'a': {
            char parse_key1[] = ",";
            
            // memory pointer for strtok_r
            char *tok_ptr1;
            
            char *ptok1 = strtok_r(optarg,parse_key1,&tok_ptr1);
            int count1 = 0;
            while( ptok1 != NULL ) {
               if ( count1 == 0 ) {
                  // see if it is built in amount
                  char ex = optarg[strlen(optarg)-1];
                  if ( ex == 'e' || ex == 'x' ) {
                     optarg[strlen(optarg)-1] = '\0';
                     double num_part = 0;
                     if ( strlen(optarg) == 0 )
                        num_part = 1.0;
                     else
                     if( !read_double(optarg, &num_part, errmsg) )
                        error(errmsg, "rotation value");
                        
                     rotation_as_increment += rad2deg(num_part);
                     if ( ex == 'x' )
                        add_pi = true;
                  }
                  else {
                     // find 'rad' in ptok1, else value is degrees
                     char *pch = strstr(ptok1,"rad");
                     bool rotation_as_inc = ( ( pch == NULL ) ? false : true );
                     double rot;
                     if(!read_double(ptok1, &rot, errmsg))
                        error(errmsg, "rotation value");
                     if ( rotation_as_inc )
                        rotation_as_increment += rot;
                     else
                        rotation += rot;
                  }
               }
               else
               if ( count1 == 1 ) {
                  double d;
                  if(!read_double(ptok1, &d, errmsg))
                     error(errmsg, "rotation axis");
                  int a = (int)floor(d);
                  //if ( rotation_axis_tmp[1] - a > 0.0 )
                  //   error(msg_str("axis numbers must be specified by an integer: '%g'", rotation_axis_tmp[i]), c);
                  rotation_axis.push_back(a);
                  
                  if( rotation_axis[0] > 2 )
                     error("ratio direction should be 0, 1 or 2", c);
               }
               
               ptok1 = strtok_r(NULL,parse_key1,&tok_ptr1);
               count1++;
            }
            
            
            break;
         }
           
         case 'r': // ratio direction and ratio
         {
            vector<double> ratio_direction_tmp;
            if(!read_double_list(optarg, ratio_direction_tmp, errmsg, 2))
               error(errmsg, c);
               
            // pull out ratio
            ratio = ratio_direction_tmp[0];
            // if zero, make a minimum ratio
            // a little lower than built in epsilon
            if ( ratio == 0.0 )
               ratio = epsilon/10.0;
            
            if ( ratio_direction_tmp.size() > 2 ) {
               error("ratio takes 1 or 2 arguments",c);
            }
            else
            if ( ratio_direction_tmp.size() == 2 ) {           
               int a = (int)floor(ratio_direction_tmp[1]);
               //if ( ratio_direction_tmp[1] - a > 0.0 )
               //   error(msg_str("axis numbers must be specified by an integer: '%g'", ratio_direction_tmp[i]), c);
               ratio_direction.push_back(a);
                  
               ratio_direction_tmp.clear();
                  
               if( ratio_direction[0] > 2 )
                  error("ratio direction should be 0, 1 or 2", c);
            }      
            break;
         }
            
         case 'C':
            id = get_arg_id(optarg, "polygons=1|suppress=2|force=3|normal=4", argmatch_add_id_maps, errmsg);
            if(id=="")
               error(errmsg,c);
            convex_hull = atoi(id.c_str());
            break;
            
         case 'v':
            verbose = true;
            break;
            
         case 'f':
            if(!strcasecmp(optarg,"none"))
               face_coloring_method = '\0';
            else
            if(strspn(optarg, "an") != strlen(optarg) || strlen(optarg)>1)
               error(msg_str("invalid face coloring method '%s'", optarg), c);
            else {
               face_coloring_method = *optarg;
            }
            break;
            
         case 'V':
            if(!vert_col.read(optarg, errmsg))
               error(errmsg, c);
            break;

         case 'E':
            if(!edge_col.read(optarg, errmsg))
               error(errmsg, c);
            break;

         case 'T':
            if(!read_int(optarg, &face_opacity, errmsg))
               error(errmsg, c);
            if(face_opacity < 0 || face_opacity > 255) {
               error("face transparency must be between 0 and 255", c);
            }
            break;

         case 'm':
            map_file = optarg;
            break;

         case 'o':
            ofile = optarg;
            break;

         default:
            error("unknown command line error");
      }
   }
   
   if(argc-optind > 0)
      error("too many arguments");
      
   if (mode == 0)
      error("one of -k, -t, -d must be specified");
      
   // if option -n was used, convert n to multipliers
   if ( (int)n.size() ) {
      multipliers.clear();
      for( int i=0; i<(int)n.size(); i++ ) {
         multipliers.push_back(n[i]);
         // twister rhomb
         if ( mode == 3 )
            multipliers[i] /= dihedral_n;
      }
   }
   
   // d must be filled in any case
   for( int i=(int)d.size(); i<2; i++ )
      d.push_back(1);
      
   if (!map_file.size())
      map_file = "m1";

   if (map_file == "m1" || map_file == "m2") {
      color_map_map *col_map1 = new color_map_map;
      color_map_map *col_map2 = new color_map_map;
      if (map_file == "m1") {
         col_map1->set_col(0, col_val(255,0,0));          // axis1 red
         col_map1->set_col(1, col_val(255,127,0));        // axis2 darkoranage1
         col_map1->set_col(2, col_val(255,255,0));        // axis3 yellow
         //col_map1->set_col(3, col_val(0,100,0));  // convex hull darkgreen
         col_map1->set_col(3, col_val(139,69,19));        // convex hull - saddlebrown
         col_map1->set_wrap();
         map.add_cmap(col_map1);
      }
      else
      if (map_file == "m2") {
         // colors from PDF document measured from screen
         col_map1->set_col(0, col_val(130,95,34));        // 3-sided faces
         col_map1->set_col(1, col_val(99,117,88));        // 4-sided faces
         col_map1->set_col(2, col_val(84,139,35));        // 5-sided faces
         col_map1->set_col(3, col_val(96,109,28));        // 6-sided faces
         col_map1->set_col(4, col_val(128,128,128));      // 7-sided faces
         col_map1->set_col(5, col_val(118,97,85));        // 8-sided faces
         map.add_cmap(col_map1);
         
         col_map2->set_col(0, col_val(128,144,79));       // 9-sided faces and higher
         col_map2->set_wrap();
         map.add_cmap(col_map2);
      }
   }
   else
   if(!map.init(map_file.c_str(), errmsg))
      error(errmsg, 'm');
}

class symmetro
{
public:
	symmetro()
	{
		for( int i=0; i<2; i++ ) {
		   mult.push_back(0);
		   sym_vec.push_back(vec3d());
	   }
	}
	
	void debug();
	void setSym( const char &s, const int &id_no, const int &psym, const int &qsym, const int &dih_n );
	void setMult( const int &a, const int &m );
	int getMult( const int &a );
	int getOrder( const int &a );
	int getN( const int &a );
	
	double getAngleBetweenAxes( const int &axis1, const int &axis2 );
	double getAngleBetweenAxesSin( const int &axis1, const int &axis2 );
	void fill_sym_vec( const symmetro_opts &opts );
	
   double angle( const int &n, const int &d );
   double circumradius( const int &n, const int &d );

   vector<col_geom_v> CalcPolygons( const symmetro_opts &opts );
	
	~symmetro()
	{}
	
private:
   char  sym;
   int   sym_id_no;
	int   p;
	int   q;
	
	int   dihedral_n;

	vector<int> mult;
   vector<vec3d> sym_vec;
};

void symmetro::debug()
{
   if ( sym == 'S')
      fprintf(stderr,"\nsymmetry = %c%d\n\n", sym, dihedral_n*2);
   else
      fprintf(stderr,"\nsymmetry = %c[%d,%d]%d\n\n", sym, p, q, sym_id_no);
   
   for( int i=0; i<2; i++ )
      fprintf(stderr,"axis %d: mult = %d\n", i, mult[i]);
   fprintf(stderr,"\n");
   
   for( int i=0; i<2; i++ )
      if ( mult[i] )
         fprintf(stderr,"axis %d polygon: %d-gon\n", i, getN(i));
   fprintf(stderr,"\n");
}

void symmetro::setSym( const char &s, const int &id_no, const int &psym, const int &qsym, const int &dih_n )
{
   sym = s;
   sym_id_no = id_no;
   
   p = psym;
   q = qsym;
   
   dihedral_n = dih_n;
}

void symmetro::setMult( const int &a, const int &m )
{
	mult[a] = m;
}

int symmetro::getMult( const int &a )
{
	return( mult[a] );
}

int symmetro::getOrder( const int &a )
{
   switch( a ) {
   case 0: return p;
   case 1: return q;
   default: return 0;
   }
}

int symmetro::getN( const int &a )
{
	return ( getOrder( a ) * mult[ a ] );
}

double symmetro::getAngleBetweenAxes( const int &axis1, const int &axis2 ) {
   return ( acos(vdot(sym_vec[axis1].unit(), sym_vec[axis2].unit())) );
}

double symmetro::getAngleBetweenAxesSin( const int &axis1, const int &axis2 ) {
   double sin_angle_between_axes = vcross(sym_vec[axis1].unit(), sym_vec[axis2].unit()).mag();
   if ( fabs(sin_angle_between_axes) > 1.0 ) {
      sin_angle_between_axes = ( sin_angle_between_axes < 0.0 ) ? -1.0 : 1.0;
   }
   //return ( asin( safe_for_trig(sin_angle_between_axes) ) ); 
   return ( asin(sin_angle_between_axes) );
}

void symmetro::fill_sym_vec( const symmetro_opts &opts ) {
   int err_no = 0; // 1 - wrong p,q  2 - wrong sym_id_no
   
   if ( sym == 'T' ) {
      if ( p == 3 && q == 3 ) { // K-H mode +120 degrees
         if ( sym_id_no == 1 ) {
            sym_vec[0] = vec3d( 1.0, 1.0, 1.0 );
            sym_vec[1] = vec3d( -1.0, -1.0, 1.0 );
         }
      }
      else
      if ( p == 3 && q == 2 ) {
         if ( sym_id_no == 1 ) {
            sym_vec[0] = vec3d( 1.0, 1.0, 1.0 );
            sym_vec[1] = vec3d( 0.0, 0.0, 1.0 );
         }
      }
      else
      if ( p == 2 && q == 2 ) {
         if ( sym_id_no == 1 ) {
            sym_vec[0] = vec3d( 0.0, 0.0, 1.0 );
            sym_vec[1] = vec3d( 1.0, 0.0, 0.0 );
         }
      }
      else
         err_no = 1;
   }
   else
   if ( sym == 'O' ) {
      if ( p == 4 && q == 4 ) {
         if ( sym_id_no == 1 ) {
            sym_vec[0] = vec3d( 0.0, 0.0, 1.0 );
            sym_vec[1] = vec3d( 1.0, 0.0, 0.0 );
         }
      }
      else
      if ( p == 4 && q == 3 ) {
         if ( sym_id_no == 1 ) {
            sym_vec[0] = vec3d( 0.0, 0.0, 1.0 );
            sym_vec[1] = vec3d( 1.0, 1.0, 1.0 );
         }
      }
      else
      if ( p == 4 && q == 2 ) {
         if ( sym_id_no == 1 ) {
            sym_vec[0] = vec3d( 0.0, 0.0, 1.0 );
            sym_vec[1] = vec3d( 0.0, 1.0, 1.0 );
         }
         else
         if ( sym_id_no == 2 ) {
            sym_vec[0] = vec3d( 0.0, 0.0, 1.0 );
            sym_vec[1] = vec3d( 1.0, 1.0, 0.0 );
         }
      }
      else
      if ( p == 3 && q == 3 ) {
         if ( sym_id_no == 1 ) {
            sym_vec[0] = vec3d( 1.0, 1.0, 1.0 );
            sym_vec[1] = vec3d( 1.0, -1.0, 1.0 );
         }
      }
      else
      if ( p == 3 && q == 2 ) {
         if ( sym_id_no == 1 ) {
            sym_vec[0] = vec3d( 1.0, 1.0, 1.0 );
            sym_vec[1] = vec3d( 0.0, -1.0, -1.0 );
         }
         else
         if ( sym_id_no == 2 ) {
            sym_vec[0] = vec3d( 1.0, 1.0, 1.0 );
            sym_vec[1] = vec3d( 1.0, 0.0, -1.0 );
         }
      }
      else
      if ( p == 2 && q == 2 ) {
         if ( sym_id_no == 1 ) {
            sym_vec[0] = vec3d( 0.0, 1.0, 1.0 );
            sym_vec[1] = vec3d( 1.0, 0.0, 1.0 );
         }
         else
         if ( sym_id_no == 2 ) {
            sym_vec[0] = vec3d( 0.0, 1.0, 1.0 );
            sym_vec[1] = vec3d( 0.0, 1.0, -1.0 );
         }
      }
      else
         err_no = 1;
   }
   else
   if ( sym == 'I' ) {
      if ( p == 5 && q == 5 ) {
         if ( sym_id_no == 1 ) {
            sym_vec[0] = vec3d( 0.0, 1.0, phi );
            sym_vec[1] = vec3d( 0.0, 1.0, -phi );
         }
      }
      else      
      if ( p == 5 && q == 3 ) {
         if ( sym_id_no == 1 ) {
            sym_vec[0] = vec3d( 0.0, 1.0, phi );
            sym_vec[1] = vec3d( 1.0, 1.0, 1.0 );
         }
         else
         if ( sym_id_no == 2 ) {
            sym_vec[0] = vec3d( 0.0, 1.0, phi );
            sym_vec[1] = vec3d( phi, -1.0/phi, 0.0 );
         }
      }
      else
      if ( p == 5 && q == 2 ) {
         if ( sym_id_no == 1 ) {
            sym_vec[0] = vec3d( 0.0, 1.0, phi );
            sym_vec[1] = vec3d( 0.0, 0.0, -1.0 );
         }
         else
         if ( sym_id_no == 2 ) {
            sym_vec[0] = vec3d( 0.0, 1.0, phi );
            sym_vec[1] = vec3d( 1.0, 1.0/phi, -phi );
         }
         else
         if ( sym_id_no == 3 ) {
            sym_vec[0] = vec3d( 0.0, 1.0, phi );
            sym_vec[1] = vec3d( 1.0, 0.0, 0.0 );
         }
      }
      else
      if ( p == 3 && q == 3 ) {     
         if ( sym_id_no == 1 ) {
            sym_vec[0] = vec3d( 1.0, 1.0, 1.0 );
            sym_vec[1] = vec3d( -1.0/phi, 0.0, -phi );
         }
         else
         if ( sym_id_no == 2 ) {
            sym_vec[0] = vec3d( 1.0, 1.0, 1.0 );
            sym_vec[1] = vec3d( 1.0, -1.0, -1.0 );
         }
      }
      else
      if ( p == 3 && q == 2 ) {
         if ( sym_id_no == 1 ) {
            sym_vec[0] = vec3d( 1.0, 1.0, 1.0 );
            sym_vec[1] = vec3d( -1.0, -1.0/phi, -phi );
         }
         else
         if ( sym_id_no == 2 ) {
            sym_vec[0] = vec3d( 1.0, 1.0, 1.0 );
            sym_vec[1] = vec3d( -1.0, 0.0, 0.0 );
         }
         else
         if ( sym_id_no == 3 ) {
            sym_vec[0] = vec3d( 1.0, 1.0, 1.0 );
            sym_vec[1] = vec3d( 1.0, -1.0/phi, -phi );
         }
         else
         if ( sym_id_no == 4 ) {
            sym_vec[0] = vec3d( 1.0, 1.0, 1.0 );
            sym_vec[1] = vec3d( 1.0, 1.0/phi, -phi );
         }
      }
      else
      if ( p == 2 && q == 2 ) {
         if ( sym_id_no == 1 ) {
            sym_vec[0] = vec3d( 0.0, 0.0, 1.0 );
            sym_vec[1] = vec3d( 1.0, 1.0/phi, phi );
         }
         else
         if ( sym_id_no == 2 ) {
            sym_vec[0] = vec3d( 0.0, 0.0, 1.0 );
            sym_vec[1] = vec3d( 1.0/phi, phi, 1.0 );
         }
         else
         if ( sym_id_no == 3 ) {
            sym_vec[0] = vec3d( 0.0, 0.0, 1.0 );
            sym_vec[1] = vec3d( phi, 1.0, 1.0/phi );
         }
         else
         if ( sym_id_no == 4 ) {
            sym_vec[0] = vec3d( 0.0, 0.0, 1.0 );
            sym_vec[1] = vec3d( 1.0, 0.0, 0.0 );
         }
      }
      else
         err_no = 1;
   }
   else
   if ( sym == 'D' ) {
      if ( p == 2 && sym_id_no <= (dihedral_n/2) ) {
         double a = sym_id_no*M_PI/dihedral_n;
         sym_vec[0] = vec3d( 1.0, 0.0, 0.0 );
         sym_vec[1] = vec3d( cos(a), sin(a), 0 );
      }
      else
      if ( p == dihedral_n && sym_id_no == 1 ) {
         sym_vec[0] = vec3d( 0.0, 0.0, 1.0 );
         sym_vec[1] = vec3d( 1.0, 0.0, 0.0 );
      }
      else
         err_no = 1;
   }
   else
   // twister_rhomb
   if ( sym == 'S' ) {
      if ( p == dihedral_n ) {
         //acos(1/tan(pi*D/N)/tan(pi*(N-D)/(2*N)))
         double n = dihedral_n;
         double d = opts.d[0];
         double a = acos(1.0/tan(M_PI*d/n)/tan(M_PI*(n-d)/(2.0*n)));
         sym_vec[0] = vec3d( 0.0, 0.0, 1.0 );
         sym_vec[1] = vec3d( sin(a), 0, cos(a) );
      }
      else
         err_no = 1;
   }

   // sym_vec will only not be set if no id_no was found
   if ( !err_no )
      err_no = ( sym_vec[0].is_set() ) ? 0 : 2 ;
   
   if ( err_no == 1 )
      opts.error(msg_str("invalid p,q values: %d,%d", p,q), 't'); 
   else
   if ( err_no == 2 )
      opts.error(msg_str("invalid symmetry id no: %d", sym_id_no), 't');
   
   return;
}

double symmetro::angle( const int &n, const int &d )
{
   return ( (2.0*M_PI*double(d)/double(n)) );
}

double symmetro::circumradius( const int &n, const int &d )
{
   double edge_len = 1.0;
   return ( edge_len / (2.0*sin(angle(n,d)/2.0)) );
}

vector<col_geom_v> symmetro::CalcPolygons( const symmetro_opts &opts )
{
   vector<double> ratios(2);
   ratios[0] = 1.0;
   ratios[1] = 1.0;
   if ( opts.ratio )
      ratios[ ( opts.ratio_direction[0] <= opts.ratio_direction[1] ) ? 0 : 1] = opts.ratio;

   vector<int> axis;
   axis.push_back(0);
   axis.push_back(1);

   if ( opts.rotation_axis[0] > opts.rotation_axis[1] ) {
      swap( axis[0], axis[1] );
   }

   double r0 = ratios[0] * circumradius( getN(axis[0]), opts.d[axis[0]] );
   double r1 = ratios[1] * circumradius( getN(axis[1]), opts.d[axis[1]] );

   //double angle_between_axes = ( twister_mode ) ? getAngleBetweenAxesSin( axis[0], axis[1] ) : getAngleBetweenAxes( axis[0], axis[1] );
   double angle_between_axes = getAngleBetweenAxesSin( axis[0], axis[1] );
   if ( opts.verbose )
      fprintf(stderr,"\nangle between axes: radians = %.17lf degrees = %.17lf\n",angle_between_axes,rad2deg(angle_between_axes));
   mat3d rot = mat3d::rot(vec3d(0, 1, 0), angle_between_axes);
   mat3d rot_inv = mat3d::rot(vec3d(0, 1, 0), -angle_between_axes);

   double ang = deg2rad( opts.rotation );
   if ( opts.rotation_as_increment )
      ang += deg2rad( opts.rotation_as_increment ) * angle(getN(axis[0]),opts.d[axis[0]])/2.0;
   if ( opts.add_pi )
      ang += M_PI;
   if ( opts.verbose )
      fprintf(stderr,"turn angle: radians = %.17lf degrees = %.17lf\n",ang,rad2deg(ang));
   
   vec3d V = mat3d::rot(vec3d(0, 0, 1), ang) * vec3d(r0, 0, 0);
   vec3d q = rot * V;
   vec3d u = rot * vec3d(0, 0, 1);
   
   double a = u[0]*u[0] + u[1]*u[1];
   double b = 2*(q[0]*u[0] + q[1]*u[1]);
   double c = q[0]*q[0] + q[1]*q[1] - r1*r1;

   double disc = b*b - 4*a*c;
   if (disc < -epsilon)
     opts.error("model is not geometrically constructible");
   else
   if (disc < 0)
     disc = 0;

   double sign_flag = -1.0;
   if ( sym == 'S' ) {
      // AR - The sign flag, which changes for the range 90 to 270 degrees, allows
      // the model to reverse, otherwise the model breaks apart in this range.
      double turn_angle_test_val = fabs(fmod(fabs(ang), 2.0*M_PI) - M_PI);
      sign_flag = (turn_angle_test_val > M_PI/2.0) ? -1.0 : 1.0;
   }
   double t = (-b + sign_flag*sqrt(disc))/(2*a);

   vec3d P = V + vec3d(0, 0, t);
   vec3d Q = rot * P;
   
   if( vdot(sym_vec[axis[0]], sym_vec[axis[1]]) > 0.0 ) {
      sym_vec[axis[1]] *= -1.0;
   }
   
   // there can only ever be 2
   vector<col_geom_v> pgeom(2);
   
   for( int i=0; i<(int)pgeom.size(); i++ ) {
      int j = axis[i];
	   int n = getN(j);
	   int d = opts.d[j];
	   
	   // handle compound polygons
      double int_part;
      double fract_part = modf((double)n, &int_part);
      bool compound = double_eq(fract_part, 0.0, epsilon) ? true : false;
      int parts = ( compound ) ? d : 1;
      double bump_ang = angle(n,d)/(double)parts;

	   if( (n > 0) && (ratios[i] > epsilon) ) {
	   	double bump_angle = 0.0;
	   	int vert_idx = 0;
	   	
	      for( int p = 0; p < parts; p++ ) {   
            for( int idx = 0; idx < n; idx++ ) {
               if ( i == 0 ) {
	               pgeom[j].add_vert( mat3d::rot(vec3d(0, 0, 1), (idx * angle(n,d)) + bump_angle) * P );
	            }
	            else
	            if ( i == 1 ) {
	               pgeom[j].add_vert( rot_inv * mat3d::rot(vec3d(0, 0, 1), (idx * angle(n,d)) + bump_angle) * Q );
	            }
            }
            
            vector<int> face;
            for( int idx = 0; idx < n; ++idx )
               face.push_back( vert_idx++ );
            pgeom[j].add_face( face );
            face.clear();
            
            bump_angle += bump_ang;
         }

	      pgeom[j].transform( mat3d::alignment(vec3d(0, 0, 1), vec3d(1, 0, 0), sym_vec[axis[0]], sym_vec[axis[1]]) );
	      if ( sym == 'S' ) {
            pgeom[j].transform( mat3d::rot(vec3d(0.0,0.0,180.0/((double)dihedral_n*2.0))) );
         }
      }
      
      // epsilon size faces are because ratio was set at 0
      if ( fabs( ratios[i] ) <= epsilon ) {
         pgeom[j].clear_all();
      }
   }
   
   return pgeom;   
}

bool is_point_on_polygon_edges(const geom_if &polygon, const vec3d &P, const double &eps)
{
   const vector<int> &face = polygon.faces()[0];
   const vector<vec3d> &verts = polygon.verts();

   bool answer = false;

   int fsz = face.size();
   for(int i=0; i<fsz; i++) {
      vec3d v1 = verts[face[i]];
      vec3d v2 = verts[face[(i+1)%fsz]];
      if ((point_in_segment(P, v1, v2, eps)).is_set()) {
         answer = true;
         break;
      }
   }

   return answer;
}

bool detect_collision( const col_geom_v &geom )
{
   const vector<vector<int> > &faces = geom.faces();
   const vector<vec3d> &verts = geom.verts();
   
   // don't test digons
   for( int i=0; i<(int)faces.size(); i++) {
      vector<int> face0 = faces[i];
      // digons won't work in plane intersection
      if ( face0.size() < 3 )
         continue;
      for( int j=i+1; j<(int)faces.size(); j++) {
         vector<int> face1 = faces[j];
         if ( face1.size() < 3 )
            continue;

         vec3d P, dir;
         if ( two_plane_intersect(  centroid(verts, face0), face_norm(verts, face0),
                                    centroid(verts, face1), face_norm(verts, face1),
                                    P, dir, epsilon ) ) {
            if ( !P.is_set() )
               continue;               
            // if two polygons intersect, see if intersection point is inside polygon
            vector<int> face_idxs;
            face_idxs.push_back(i);
            col_geom_v polygon = faces_to_geom(geom, face_idxs);
            int winding_number = 0;
            // get winding number, if not zero, point is on a polygon
            wn_PnPoly( polygon, P, 2, winding_number, epsilon );
            // if point in on an edge set winding number back to zero
            if ( winding_number ) {
               if ( is_point_on_polygon_edges( polygon, P, epsilon ) )
                  winding_number = 0;
            }
            if ( winding_number ) {
               return true;
            }
         }
      }
   }
   
   return false;
}

col_geom_v build_geom(vector<col_geom_v> &pgeom, const symmetro_opts &opts)
{
   col_geom_v geom;
   
   for( int i=0; i<2; i++ ) {
      // if not polygon, repeat for symmetry type
      if ( opts.convex_hull > 1 ) {
         sch_sym sym; 
         if ( opts.sym == 'T' )
            sym.init(sch_sym::T);
         else
         if ( opts.sym == 'O' )
            sym.init(sch_sym::O);
         else
         if ( opts.sym == 'I' )
            sym.init(sch_sym::I);
         else
         if ( opts.sym == 'D' )
            sym.init(sch_sym::D, opts.dihedral_n);
         else
         if ( opts.sym == 'S' )
            sym.init(sch_sym::S, opts.dihedral_n*2);
         sym_repeat(pgeom[i], pgeom[i], sym, ELEM_FACES);
      }
      
      if (opts.face_coloring_method == 'a') {
         coloring clrng(&pgeom[i]);
         col_val col = opts.map.get_col(opts.col_axis_idx[i]);
         if (col.is_val())
            col = col_val(col[0],col[1],col[2],opts.face_opacity);
         clrng.f_one_col(col);
      }
      
      geom.append(pgeom[i]);
   }
   
   if ( opts.convex_hull > 1 )
      sort_merge_elems(geom, "vf", epsilon);
   
   // check for collisions
   bool collision = false;
   if ( opts.convex_hull > 2 )
      collision = detect_collision( geom );
      if ( collision ) {
         opts.warning("collision detected. convex hull is suppressed", 'C');
   }
   
   if ( ( !collision && opts.convex_hull == 4 ) || ( opts.convex_hull == 3 ) ) {
      char errmsg[MSG_SZ];
      int ret = geom.add_hull("",errmsg);
      // probably never happen
      if(!ret)
         if (opts.verbose)
            opts.warning(errmsg, 'C');

      // merged faces will retain RGB color
      sort_merge_elems(geom, "f", epsilon);

      // after sort merge, only new faces from convex hull will be uncolored
      coloring clrng(&geom);
      col_val convex_hull_color;
      if (opts.face_coloring_method == 'a')
         convex_hull_color = opts.map.get_col(3);
         
      for( int i=0; i<(int)geom.faces().size(); i++ ) {
         col_val col = geom.get_f_col(i);
         if (!col.is_set()) {
            col = convex_hull_color;
            col = col_val(col[0],col[1],col[2],opts.face_opacity);
            geom.set_f_col(i, col);
         }
      }
   }
   
   if (opts.face_coloring_method == 'n') {
      const vector<vector<int> > &faces = geom.faces();
      for (unsigned int i=0;i<faces.size();i++) {
         int fsz = faces[i].size() - 3;
         col_val col = opts.map.get_col(fsz);
         if (col.is_val())
            col = col_val(col[0],col[1],col[2],opts.face_opacity);
         geom.set_f_col(i,col);
      }
   }
   
   // color vertices and edges
   geom.color_vef(opts.vert_col, opts.edge_col, col_val());
   
   geom.orient();
   
   return geom;
}

int main(int argc, char *argv[])
{
   symmetro_opts opts;
   opts.process_command_line(argc, argv);
   
   symmetro s;
   s.setSym( opts.sym, opts.sym_id_no, opts.p, opts.q, opts.dihedral_n );
   
   // set multipliers in object
   int j = 0;
   for( int i=0; i<(int)opts.multipliers.size(); i++ ) {
      if ( opts.multipliers[i] ) {
         opts.col_axis_idx.push_back(i);
         s.setMult( j, opts.multipliers[i] );
         j++;
      }
   }
   // if only one multiplier, duplicate it
   if ( j == 1 ) {
      s.setMult( 1, s.getMult(0) );
      opts.col_axis_idx.push_back(opts.col_axis_idx[0]);
   } 

   // fill symmetry axes here      
   s.fill_sym_vec( opts );
   
   // if convex_hull is not set
   if ( !opts.convex_hull ) {
      for( int i=0; i<(int)opts.d.size(); i++ ) {
         if ( opts.d[i] > 1 ) {
            // supress convex hull
            opts.convex_hull = 2;
            opts.warning("star polygons detected so convex hull is supressed", 'C');
            break;
         }
      }
   }
   // if still not set, convex hull is set to normal
   if ( !opts.convex_hull )
      opts.convex_hull = 4;

   // check ratio axis specifier for zero
   for( int i=0; i<(int)opts.ratio_direction.size(); i++ ) {
      if ( ( opts.ratio_direction[i] >= (int)opts.multipliers.size() ) || ( opts.multipliers[opts.ratio_direction[i]] == 0 ) )
         opts.error(msg_str("polygon '%d' is not generated so cannot be used for scaling", opts.ratio_direction[i]), 'S');
   }
     
   // if empty, fill ratio direction
   if ( !opts.ratio_direction.size() ) {
      int j = 0;
      for( int i=0; i<(int)opts.multipliers.size(); i++ ) {
         if ( j == 2 )
            continue;
         if ( opts.multipliers[i] ) {
            opts.ratio_direction.push_back(i);
            j++;
         }            
      }
   }
   
   // must fill in second ratio direction for CalcPolygons (used for reference)
   if ( opts.multipliers.size() == 1 ) {
      opts.ratio_direction.push_back( opts.ratio_direction[0] );
   }
   else
   if ( opts.ratio_direction.size() == 1 ) {
     for( int i=0; i<(int)opts.multipliers.size(); i++ ) {
         if ( ( opts.multipliers[i] != 0 ) && ( opts.ratio_direction[0] != i ) ) {
            opts.ratio_direction.push_back(i);
            break;
         }
      }
   }   
   
   // check rotation axis specifier for zero
   for( int i=0; i<(int)opts.rotation_axis.size(); i++ ) {
      if ( ( opts.rotation_axis[i] >= (int)opts.multipliers.size() ) || ( opts.multipliers[opts.rotation_axis[i]] == 0 ) )
         opts.error(msg_str("polygon '%d' is not generated so cannot be used for rotation", opts.rotation_axis[i]), 'r');
   } 

   // if empty, fill rotations   
   if ( !opts.rotation_axis.size() ) {
      int j = 0;
      for( int i=0; i<(int)opts.multipliers.size(); i++ ) {
         if ( j == 2 )
            continue;
         if ( opts.multipliers[i] ) {
            opts.rotation_axis.push_back(i);
            j++;
         }            
      }
   }
   
   // must fill in second rotation for CalcPolygons (used for reference)
   if ( opts.multipliers.size() == 1 ) {
      opts.rotation_axis.push_back( opts.rotation_axis[0] );
   }
   else
   if ( opts.rotation_axis.size() == 1 ) {
     for( int i=0; i<(int)opts.multipliers.size(); i++ ) {
         if ( opts.rotation_axis[0] != i ) {
            opts.rotation_axis.push_back(i);
            break;
         }
      }
   }
   
   vector<col_geom_v> pgeom = s.CalcPolygons( opts );
   
   if ( opts.verbose ) {
      s.debug();
      
      double edge_length[2];
      for( int i=0; i<(int)pgeom.size(); i++ ) {
         geom_info info(pgeom[i]);
         if (info.num_iedges() > 0) {
            edge_length[i] = info.iedge_lengths().sum/info.num_iedges();
            fprintf(stderr,"Edge length of polygon %d = %.17lf\n", i, edge_length[i]);
         }
      }
      
      fprintf(stderr,"\n");
      for( int i=0; i<2; i++ ) {
         for( int j=0; j<2; j++ ) {
            if (i==j)
               continue;
            if ( edge_length[i] > epsilon && edge_length[j] > epsilon )
               fprintf(stderr,"edge length ratio of polygon %d to %d = %.17lf\n", i, j, edge_length[i] / edge_length[j] );
         }
      }
      
      fprintf(stderr,"\n");
   }
   
   col_geom_v geom;
   geom = build_geom(pgeom, opts);
   
   /* for looking at the 3/2
   vector<vec3d> &verts = geom.raw_verts();
   for( int i=0; i<(int)verts.size(); i++ ) {
      vec3d v = verts[i];
      for( int j=0; j<3; j++ ) {
         if ( v[j] > 1000000.0 || isnan(v[j]) ) {
            v[j] = 0.0;
            verts[i] = v;
         }
      }
      verts[i].dump();
   }
   */
   
   geom_write_or_error(geom, opts.ofile, opts);
   
   return 0;
}

