/** \HEADER
 *************************************************************************
 *
 *                            Kokkos
 *                 Copyright 2010 Sandia Corporation
 *
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the Corporation nor the names of the
 *  contributors may be used to endorse or promote products derived from
 *  this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************
 */

#include <map>
#include <ostream>
#include <Kokkos_CudaDevice.hpp>

namespace Kokkos {
namespace {

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// For the cuda device singleton.

class CudaDeviceImpl {
public:
  std::map<void*,std::string> m_allocations ;

  // Appropriate cached device information

  CudaDeviceImpl();

  static CudaDeviceImpl & singleton();
};

CudaDeviceImpl::CudaDeviceImpl()
  : m_allocations()
{
  // Appropriate device queries

}

CudaDeviceImpl & CudaDeviceImpl::singleton()
{
  static CudaDeviceImpl * impl = NULL ;
  if ( impl == NULL ) { impl = new CudaDeviceImpl(); }
  return *impl ;
}

}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

__global__
void clear_memory( int * ptr , int count )
{
  const int n = count ;
  const int s = blockDim.x * gridDim.x ;

  int i = threadIdx.x + blockDim.x * blockIdx.x ;
  
  for ( ; i < n ; i += s ) ptr[i] = 0 ;
}

void * CudaDevice::allocate_memory( size_type member_size ,
                                    size_type member_count ,
                                    const std::string & label )
{
  int * ptr_on_device = NULL ;

  dim3 dimGrid(  256 , 1 , 1 );
  dim3 dimBlock( 256 , 1 , 1 );

  // Require member_size be a multiple of word size?

  cudaMalloc( & ptr_on_device , member_size * member_count );

  clear_memory<<< dimGrid , dimBlock >>>( ptr_on_device , member_size * member_count / sizeof(unsigned) );

  CudaDeviceImpl::singleton().m_allocations[ ptr_on_device ] = label ;

  return ptr_on_device ;
}

void CudaDevice::deallocate_memory( void * ptr_on_device )
{
  CudaDeviceImpl::singleton().m_allocations.erase( ptr_on_device );

  cudaFree( ptr_on_device );
}

void CudaDevice::print_allocations( std::ostream & s ) const
{
  CudaDeviceImpl & impl = CudaDeviceImpl::singleton() ;

  std::map<void*,std::string>::const_iterator i = impl.m_allocations.begin();
  std::map<void*,std::string>::const_iterator end = impl.m_allocations.end();

  for ( ; i != end ; ++i ) {
    s << i->second << std::endl ;
  }
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

} // namespace Kokkos



