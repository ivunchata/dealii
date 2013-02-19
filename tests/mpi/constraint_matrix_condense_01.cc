//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2013 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


// check ConstraintMatrix::condense(Vector) for parallel vectors. this
// function used to crash
//
// original test case by Daniel Arndt

#include "../tests.h"
#include <deal.II/base/logstream.h>
#include <deal.II/lac/petsc_parallel_vector.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/dofs/dof_tools.h>
#include <deal.II/fe/fe_q.h>
#include <fstream>


void test ()
{
  MPI_Comm mpi_communicator (MPI_COMM_WORLD);
  parallel::distributed::Triangulation<3> triangulation(mpi_communicator);

  FE_Q<3>          fe(1);

  DoFHandler<3>    dof_handler(triangulation);
  GridGenerator::hyper_cube (triangulation);
  triangulation.refine_global (1);

  dof_handler.distribute_dofs (fe);

  IndexSet           locally_owned_dofs;
  locally_owned_dofs = dof_handler.locally_owned_dofs();
  IndexSet           locally_relevant_dofs;
  DoFTools::extract_locally_relevant_dofs (dof_handler,locally_relevant_dofs);

  PETScWrappers::MPI::Vector force;
  force.reinit (mpi_communicator, locally_owned_dofs);
  Assert(!force.has_ghost_elements(), ExcInternalError());

  ConstraintMatrix constraints (locally_relevant_dofs);
  constraints.clear();
  {
    IndexSet boundary_dofs (dof_handler.n_dofs());
    DoFTools::extract_boundary_dofs
      (dof_handler, std::vector<bool>(1,true),boundary_dofs);

    unsigned int first_nboundary_dof = 0;
    while(boundary_dofs.is_element(first_nboundary_dof))
      first_nboundary_dof++;

    if (locally_relevant_dofs.is_element(first_nboundary_dof))
      {
	constraints.add_line (first_nboundary_dof);
	for (unsigned int i=0; i<dof_handler.n_dofs();++i)
	  if (boundary_dofs.is_element(i) == true)
	    constraints.add_entry (first_nboundary_dof, i, -1);
      }
  }
  constraints.close();

  constraints.condense(force);
}


int main(int argc, char *argv[])
{
  Utilities::MPI::MPI_InitFinalize mpi (argc, argv);
  PetscInitialize(&argc,&argv,0,0);

  unsigned int myid = Utilities::MPI::this_mpi_process (MPI_COMM_WORLD);

  if (myid == 0)
    {
      std::ofstream logfile(output_file_for_mpi("constraint_matrix_condense_01").c_str());
      deallog.attach(logfile);
      deallog << std::setprecision(4);
      deallog.depth_console(0);
      deallog.threshold_double(1.e-10);

      test();

      deallog << "OK" << std::endl;
    }
  else
    test();

  PetscFinalize();
  return 0;
}