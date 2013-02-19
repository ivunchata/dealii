/* $Id:  $
 *
 * Copyright (C) 2013 by the deal.II authors
 *
 * This file is subject to QPL and may not be  distributed
 * without copyright and license information. Please refer
 * to the file deal.II/doc/license.html for the  text  and
 * further information on this license.
 */

// @sect3{Include files}

#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/grid/tria_boundary_lib.h>
#include <deal.II/grid/grid_out.h>
#include <deal.II/grid/grid_in.h>

#include <fstream>

#include <map>

using namespace dealii;

// @sect3{Generate some output for a given mesh}

template<int dim>
void mesh_info(Triangulation<dim> &tria, const char *filename)
{
  // some general info:
  std::cout << "Mesh info:" << std::endl
            << " dimension: " << dim << std::endl
            << " no. of cells: " << tria.n_active_cells() << std::endl;

  // loop over all the cells and find how often each boundary indicator is used:
  {
    std::map<unsigned int, unsigned int> boundary_count;
    typename Triangulation<dim>::active_cell_iterator
    cell = tria.begin_active(),
    endc = tria.end();
    for (; cell!=endc; ++cell)
      {
        for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
          {
            if (cell->face(face)->at_boundary())
              boundary_count[cell->face(face)->boundary_indicator()]++;
          }
      }

    std::cout << " boundary indicators: ";
    for (std::map<unsigned int, unsigned int>::iterator it=boundary_count.begin();
         it!=boundary_count.end();
         ++it)
      {
        std::cout << it->first << "(" << it->second << ") ";
      }
    std::cout  << std::endl;
  }

  // Now we want to write a graphical representation of the mesh to an output
  // file.
  std::ofstream out (filename);
  GridOut grid_out;
  grid_out.write_eps (tria, out);
  std::cout << " written to " << filename << std::endl;
}

// @sect3{Main routines}


// load from a msh generated by gmsh
void grid_1 ()
{
  Triangulation<2> triangulation;

  GridIn<2> gridin;
  gridin.attach_triangulation(triangulation);
  std::ifstream f("untitled.msh");
  gridin.read_msh(f);

  mesh_info(triangulation, "grid-1.eps");

}


// merge triangulations
// note: the vertices have to be exactly on top of each other!
void grid_2 ()
{
  Triangulation<2> triangulation;

  Triangulation<2> tria1, tria2;


  GridGenerator::hyper_cube_with_cylindrical_hole (tria1, 0.25, 1.0);
  std::vector< unsigned int > repetitions(2);
  repetitions[0]=3;
  repetitions[1]=2;
  GridGenerator::subdivided_hyper_rectangle (tria2, repetitions,
                                             Point<2>(1.0,-1.0), Point<2>(4.0,1.0));

  GridGenerator::merge_triangulations (tria1, tria2, triangulation);

  mesh_info(triangulation, "grid-2.eps");
}

// move vertices
void grid_3 ()
{
  Triangulation<2> triangulation;
  GridGenerator::hyper_cube_with_cylindrical_hole (triangulation, 0.25, 1.0);

  Triangulation<2>::active_cell_iterator
  cell = triangulation.begin_active(),
  endc = triangulation.end();
  for (; cell!=endc; ++cell)
    {
      for (unsigned int i=0; i<GeometryInfo<2>::vertices_per_cell; ++i)
        {
          Point<2> &v = cell->vertex(i);
          if (std::abs(v(1)-1.0)<1e-5)
            v(1)+=0.5;
        }

    }

  // here we are going to set a boundary descriptor for the round hole
  // and refine the mesh twice.
  const HyperBallBoundary<2> boundary_description(Point<2>(0,0),0.25);
  triangulation.set_boundary (1, boundary_description);
  triangulation.refine_global(2);

  mesh_info(triangulation, "grid-3.eps");

  // remove boundary object from Triangulation again:
  triangulation.set_boundary (1);
}

// demonstrate extrude_triangulation
void grid_4()
{
  Triangulation<2> triangulation;
  Triangulation<3> out;
  GridGenerator::hyper_cube_with_cylindrical_hole (triangulation, 0.25, 1.0);

  GridGenerator::extrude_triangulation(triangulation, 3, 2.0, out);
  mesh_info(out, "grid-4.eps");
  
}

struct Grid5Func
{
    Point<2> operator() (const Point<2> & in) const
      {
	return Point<2>(in(0), in(1)+sin(in(0)/5.0*3.14159));
      }      
};


// demonstrate GridTools::transform
void grid_5()
{
  Triangulation<2> tria;
  std::vector< unsigned int > repetitions(2);
  repetitions[0]=14;
  repetitions[1]=2;
  GridGenerator::subdivided_hyper_rectangle (tria, repetitions,
                                             Point<2>(0.0,0.0), Point<2>(10.0,1.0));

  
  GridTools::transform(Grid5Func(), tria);
  mesh_info(tria, "grid-5.eps");  
}

struct Grid6Func
{
    double trans(double x) const
      {
	return tanh(2*x)/tanh(2);
      }
    
    Point<2> operator() (const Point<2> & in) const
      {
	return Point<2>((in(0)), trans(in(1)));
      }      
};
  

// demonstrate GridTools::transform
void grid_6()
{
  Triangulation<2> tria;
  std::vector< unsigned int > repetitions(2);
  repetitions[0]=40;
  repetitions[1]=40;
  GridGenerator::subdivided_hyper_rectangle (tria, repetitions,
                                             Point<2>(0.0,0.0), Point<2>(1.0,1.0));

  GridTools::transform(Grid6Func(), tria);
  mesh_info(tria, "grid-6.eps");  
}



//demonstrate distort_random
void grid_7()
{
  Triangulation<2> tria;
  std::vector< unsigned int > repetitions(2);
  repetitions[0]=16;
  repetitions[1]=16;
  GridGenerator::subdivided_hyper_rectangle (tria, repetitions,
                                             Point<2>(0.0,0.0), Point<2>(1.0,1.0));

  tria.distort_random(0.3, true);
  mesh_info(tria, "grid-7.eps");  
}

// @sect3{The main function}

// Finally, the main function. There isn't much to do here, only to call the
// subfunctions.
int main ()
{
  grid_1 ();
  grid_2 ();
  grid_3 ();
  grid_4 ();
  grid_5 ();
  grid_6 ();
  grid_7 ();
}