// Copyright (c) 2005  INRIA (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $Source$
// $Revision$
// $Name$
//
// Author(s)     : Laurent Saboret, Bruno Levy, Pierre Alliez


#ifndef MEAN_VALUE_COORDINATES_PARAMETIZER_3_H
#define MEAN_VALUE_COORDINATES_PARAMETIZER_3_H

#include "CGAL/Fixed_border_parametizer_3.h"
#include "CGAL/parameterization_assertions.h"

CGAL_BEGIN_NAMESPACE


// Class Mean_value_coordinates_parametizer_3
// Model of the Parametizer_3 concept
// Implement Floater's Mean Value Coordinates parameterization. 1 to 1 mapping is guaranteed if surface's border is mapped onto a convex polygon.
// This is a conformal parameterization, i.e. it attempts to preserve angles. 
template <class MeshAdaptor_3,														// 3D surface
		  class BorderParametizer_3 = Circular_border_parametizer_3<MeshAdaptor_3>,	// Class to map the surface's border onto a 2D space
		  class SparseLinearAlgebraTraits_d = OpenNL::DefaultLinearSolverTraits<typename MeshAdaptor_3::NT> >	
																					// Traits class for solving a sparse linear system "A*X = B"
class Mean_value_coordinates_parametizer_3 
	: public Fixed_border_parametizer_3<MeshAdaptor_3, BorderParametizer_3, SparseLinearAlgebraTraits_d>
{
// Public types
public:
				// Export Mesh_Adaptor_3, BorderParametizer_3 and SparseLinearAlgebraTraits_d types and subtypes
				typedef MeshAdaptor_3													Mesh_adaptor_3;
				typedef typename Parametizer_3<MeshAdaptor_3>::ErrorCode				ErrorCode;
				typedef typename MeshAdaptor_3::NT										NT;
				typedef typename MeshAdaptor_3::Face									Face;
				typedef typename MeshAdaptor_3::Vertex									Vertex;
				typedef typename MeshAdaptor_3::Point_3									Point_3;
				typedef typename MeshAdaptor_3::Point_2									Point_2;
				typedef typename MeshAdaptor_3::Vector_3								Vector_3;
				typedef typename MeshAdaptor_3::Face_iterator							Face_iterator;
				typedef typename MeshAdaptor_3::Face_const_iterator						Face_const_iterator;
				typedef typename MeshAdaptor_3::Vertex_iterator							Vertex_iterator;
				typedef typename MeshAdaptor_3::Vertex_const_iterator					Vertex_const_iterator;
				typedef typename MeshAdaptor_3::Border_vertex_iterator					Border_vertex_iterator;
				typedef typename MeshAdaptor_3::Border_vertex_const_iterator			Border_vertex_const_iterator;
				typedef typename MeshAdaptor_3::Vertex_around_face_circulator			Vertex_around_face_circulator;
				typedef typename MeshAdaptor_3::Vertex_around_face_const_circulator		Vertex_around_face_const_circulator;
				typedef typename MeshAdaptor_3::Vertex_around_vertex_circulator			Vertex_around_vertex_circulator;
				typedef typename MeshAdaptor_3::Vertex_around_vertex_const_circulator	Vertex_around_vertex_const_circulator;
				typedef BorderParametizer_3												Border_parametizer_3;
				typedef SparseLinearAlgebraTraits_d										Sparse_linear_algebra_traits_d;
				typedef typename SparseLinearAlgebraTraits_d::Vector					Vector;
				typedef typename SparseLinearAlgebraTraits_d::Matrix					Matrix;

// Public operations
public:
				// Constructor
				// @param borderParametizer	Object that maps the surface's border onto a 2D space
				// @param linearAlgebra		Traits object to access the "A*X = B" sparse linear system used by parameterization algorithms
				Mean_value_coordinates_parametizer_3 (BorderParametizer_3 borderParametizer = BorderParametizer_3(), 
													  SparseLinearAlgebraTraits_d linearAlgebra = SparseLinearAlgebraTraits_d()) 
				:	Fixed_border_parametizer_3<MeshAdaptor_3, BorderParametizer_3, SparseLinearAlgebraTraits_d>(borderParametizer, linearAlgebra)
				{}

				// Default copy constructor and operator =() are fine

// Protected stuff
protected:
				// compute wij = (i,j) coefficient of matrix A for j neighbor vertex of i 
				virtual	NT  compute_wij(const MeshAdaptor_3& mesh, const Vertex& main_vertex_Vi, Vertex_around_vertex_const_circulator neighbor_vertex_Vj)
				{
					Point_3	position_Vi = mesh.get_vertex_position(main_vertex_Vi);
					Point_3	position_Vj = mesh.get_vertex_position(*neighbor_vertex_Vj);

					// Compute the norm of Vj -> Vi vector
					Vector_3 edge = position_Vi - position_Vj;
					double len = std::sqrt(edge*edge);

					// Compute angle of corner specified by Vj,Vi,Vk points (ie tangent of Vi corner)
					// if Vk is the vertex before Vj when circulating around Vi
					Vertex_around_vertex_const_circulator previous_vertex_Vk = neighbor_vertex_Vj; previous_vertex_Vk --;
					Point_3	position_Vk = mesh.get_vertex_position(*previous_vertex_Vk);
					double gamma_ij  = compute_angle_rad(position_Vj, position_Vi, position_Vk);

					// Compute angle of corner specified by Vl,Vi,Vj points (ie tangent of Vi corner)
					// if Vl is the vertex after Vj when circulating around Vi
					Vertex_around_vertex_const_circulator next_vertex_Vl = neighbor_vertex_Vj; next_vertex_Vl ++;
					Point_3	position_Vl = mesh.get_vertex_position(*next_vertex_Vl);
					double delta_ij = compute_angle_rad(position_Vl, position_Vi, position_Vj);

					double weight = 0.0;
					assert(len != 0.0);												// 2 points are identical!
					if(len != 0.0)
						weight = (std::tan(0.5*gamma_ij) + std::tan(0.5*delta_ij)) / len;
					assert(weight > 0);

					return weight;
				}

				// Check if 3D -> 2D mapping is 1 to 1
				virtual bool  is_one_to_one_mapping (const MeshAdaptor_3& mesh, const Solver& solver_u, const Solver& solver_v)
				{
					// Theorem: 1 to 1 mapping is guaranteed if all Wij coefficients are > 0 (for j vertex neighbor of i)
					//          and if the surface boundary is mapped onto a 2D convex polygon
					// Floater formula above implies that Wij > 0 (for j vertex neighbor of i), thus  
					// mapping is guaranteed if the surface boundary is mapped onto a 2D convex polygon
					return get_border_parametizer().is_border_convex ();
				}
};


CGAL_END_NAMESPACE

#endif //MEAN_VALUE_COORDINATES_PARAMETIZER_3_H
