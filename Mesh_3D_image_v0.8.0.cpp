#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>

#include <CGAL/Labeled_image_mesh_domain_3.h>
#include <CGAL/Mesh_domain_with_polyline_features_3.h>

#include <CGAL/make_mesh_3.h>
#include <CGAL/Image_3.h>

//Sizing fields
#include <Sizing_fields.h> 
//Input parameters
#include <input_parameters.h>

// Domain
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Labeled_image_mesh_domain_3<CGAL::Image_3,K> Mesh_domain;

//For inexact functions
typedef K::FT FT;
typedef K::Point_3 Point;
typedef K::Vector_3 Vector;

// Polyline
typedef std::vector<Point>        Polyline_3;
typedef std::list<Polyline_3>       Polylines;

// Triangulation
typedef CGAL::Mesh_triangulation_3<Mesh_domain>::type Tr;
typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr> C3t3;

// Criteria
typedef CGAL::Mesh_criteria_3<Tr> Mesh_criteria;

// To avoid verbose function and named parameters call
using namespace CGAL::parameters;
using namespace std;

int main()
{
  // Loads image
  CGAL::Image_3 image;

  // Read input file with parameters
  Input p;
  p.load_file();
  
  // Reading image file
  std::cout<<"\n Reading the Image file... ";
  image.read("input.inr");
  
  // Domain
  Mesh_domain domain(image);
  
  //Define planar Sizing field with electrodes
  
  FT h, ub; 
  if(p.direction==1) {h=p.height*image.vx ();ub=image.vx () * image.xdim ();}
  else if(p.direction==2) {h=p.height*image.vy ();ub=image.vy () * image.ydim ();}
  else if(p.direction==3) {h=p.height*image.vz ();ub=image.vz () * image.zdim ();}
  else {std::cout<<"wrong direction index, sizer will use constant fine size everywhere"; h=1;ub=1;}

  sizing_field_planar size_p (h,p.direction,ub);
  size_p.coarse_size=p.ccs;
  size_p.fine_size=p.cs;
  size_p.preserve=p.pres; //Planar gradient

  
    
  // Mesh criteria: faces and cells
  Mesh_criteria criteria(facet_angle=p.fa, facet_size=size_p, facet_distance=p.fd,
                         cell_radius_edge_ratio=p.cre, cell_size=size_p);
  

  // Meshing
  std::cout<<"\n Meshing with initial mesh...";
  C3t3 c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria, CGAL::parameters::features(domain), 
						CGAL::parameters::no_lloyd(), CGAL::parameters::no_odt(), 
						CGAL::parameters::no_perturb(),CGAL::parameters::no_exude());
	//can be actually simplyfied: 
	//C3t3 c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria);*/
  
  //Optimisation
  std::cout<<"\n Optimising: ";
  if (p.if_perturb==1)  {std::cout<<"\n Perturb... "; CGAL::perturb_mesh_3(c3t3, domain,sliver_bound=10, time_limit=p.time_lim);}
  if (p.if_lloyd==1)  {std::cout<<"\n Lloyd... ";CGAL::lloyd_optimize_mesh_3(c3t3, domain, time_limit=p.time_lim);}
  if (p.if_odt==1)  {std::cout<<"\n ODT... "; CGAL::odt_optimize_mesh_3(c3t3, domain, time_limit=p.time_lim);}
  if (p.if_exude==1)  {std::cout<<"\n Exude... "; CGAL::exude_mesh_3(c3t3, sliver_bound=10, time_limit=p.time_lim);}
 
  // Output
  std::cout<<"\n Saving the mesh... ";
  std::ofstream medit_file("out.mesh");
  c3t3.output_to_medit(medit_file);
  medit_file.close();
  
  std::cout<<"\n ALL DONE, press any key! :)";
  getchar();

  return 0;
}
