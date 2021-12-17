/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#ifndef Akri_CDFEM_Options_SierraParser_h
#define Akri_CDFEM_Options_SierraParser_h

#include <parser/Prsr_Parser.h>
#include <parser/Prsr_Command.h>

#include <memory>

namespace krino { class CDFEM_Support; }

namespace krino {

class CDFEM_Options_SierraParser : public Prsr_CommandBlock  {
public:

  CDFEM_Options_SierraParser();
  virtual ~CDFEM_Options_SierraParser();

  static const char * usage;
  static const char * physics;

  static CDFEM_Options_SierraParser & self();
  const char * class_name() const ;

  bool prsr_register_commands(Prsr_CommandBlock &);

  static sierra::class_tag * prsr_handler_begin( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_end( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_cdfem_edge_tol( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_cdfem_dof_edge_tol( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_cdfem_internal_side_stabilization( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_num_initial_decomposition_cycles( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_cdfem_edge_degeneracy_handling( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_cdfem_nonconformal_adaptivity_levels( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_cdfem_nonconformal_adaptivity_target_elem_count( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_post_adaptivity_refinement_levels( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_cdfem_prolongation_model( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_use_hierarchical_dofs( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_constrain_CDFEM_to_XFEM_space( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_cdfem_element_size( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_initial_prolongation_field( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_use_ale_prolongation(const Prsr_CommandValues &);
  static sierra::class_tag * prsr_handler_simplex_generation_method( const Prsr_CommandValues & );

private:
  static CDFEM_Support & cdfem_support() { ThrowAssert(nullptr != self().my_cdfem_support); return *self().my_cdfem_support; }

private:
  CDFEM_Support * my_cdfem_support;
};

} // namespace krino

#endif // Akri_CDFEM_Options_SierraParser_h
