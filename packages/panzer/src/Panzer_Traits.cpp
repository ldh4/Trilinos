// @HEADER
// @HEADER

// ******************************************************************
// ******************************************************************
// Debug strings.  Specialize the Evaluation and Data types for the
// TypeString object in the phalanx/src/Phalanx_TypeStrings.hpp file.
// ******************************************************************
// ******************************************************************

#include <string>
#include "Panzer_Traits.hpp"

const std::string PHX::TypeString<panzer::Traits::Residual>::value = "Residual";

const std::string PHX::TypeString<panzer::Traits::Jacobian>::value = "Jacobian";

const std::string PHX::TypeString<double>::value = "double";

const std::string PHX::TypeString< Sacado::Fad::DFad<double> >::value = 
  "Sacado::Fad::DFad<double>";

#ifdef HAVE_STOKHOS

   const std::string PHX::TypeString<panzer::Traits::SGResidual>::value = "SGJacobian";

   const std::string PHX::TypeString<panzer::Traits::SGJacobian>::value = "SGJacobian";

   const std::string PHX::TypeString<panzer::Traits::SGType>::value = "SGType";

   const std::string PHX::TypeString<panzer::Traits::SGFadType>::value = "Sacado::Fad::DFad<SGType>";

#endif
