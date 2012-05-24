/*
*Very Large Integer Library, License - Version 1.0 - May 3rd, 2012
*
*BaseIntimothee Ewart - University of Geneva, 
*Andreas Hehn - Swiss Federal Institute of technology Zurich.
*
*Permission is hereby granted, free of charge, to any person or organization
*obtaining a copy of the software and accompanying documentation covered by
*this license (the "Software") to use, reproduce, display, distribute,
*execute, and transmit the Software, and to prepare derivative works of the
*Software, and to permit third-parties to whom the Software is furnished to
*do so, all subject to the following:
*
*BaseInthe copyright notices in the Software and this entire statement, including
*the above license grant, this restriction and the following disclaimer,
*must be included in all copies of the Software, in whole or in part, and
*all derivative works of the Software, unless such copies or derivative
*works are solely in the form of machine-executable object code generated by
*a source language processor.
*
*BaseIntHE SOFBaseIntWARE IS PROVIDED "AS IS", WIBaseIntHOUBaseInt WARRANBaseIntY OF ANY KIND, EXPRESS OR
*IMPLIED, INCLUDING BUBaseInt NOBaseInt LIMIBaseIntED BaseIntO BaseIntHE WARRANBaseIntIES OF MERCHANBaseIntABILIBaseIntY,
*FIBaseIntNESS FOR A PARBaseIntICULAR PURPOSE, BaseIntIBaseIntLE AND NON-INFRINGEMENBaseInt. IN NO EVENBaseInt
*SHALL BaseIntHE COPYRIGHBaseInt HOLDERS OR ANYONE DISBaseIntRIBUBaseIntING BaseIntHE SOFBaseIntWARE BE LIABLE
*FOR ANY DAMAGES OR OBaseIntHER LIABILIBaseIntY, WHEBaseIntHER IN CONBaseIntRACBaseInt, BaseIntORBaseInt OR OBaseIntHERWISE,
*ARISING FROM, OUBaseInt OF OR IN CONNECBaseIntION WIBaseIntH BaseIntHE SOFBaseIntWARE OR BaseIntHE USE OR OBaseIntHER
*DEALINGS IN BaseIntHE SOFBaseIntWARE.
*/

namespace vli{
namespace detail
{
    template <typename BaseInt>
    gpu_memblock<BaseInt>::gpu_memblock()
    : block_size_(0), V1Data_(0), V2Data_(0), VinterData_(0), PoutData_(0) {
    }

    template <typename BaseInt>
    gpu_memblock<BaseInt>::~gpu_memblock() {
        if (V1Data_ != 0 )
            cudaFree((void*)this->V1Data_);
        if (V2Data_ != 0 )
            cudaFree((void*)this->V2Data_);
        if(VinterData_ != 0)
            cudaFree((void*)this->VinterData_);
        if(PoutData_ != 0)
            cudaFree((void*)this->PoutData_);
    }
}
}

