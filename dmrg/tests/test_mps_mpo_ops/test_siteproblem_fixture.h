/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.
 *            See LICENSE.txt for details.
 */

#ifndef TEST_SITEPROBLEM_FIXTURE_H
#define TEST_SITEPROBLEM_FIXTURE_H

#include "maquis_dmrg.h"

/** @brief Fixture with the objects used in the SiteProblem tests */
class TestSiteproblemFixture
{
public:
    using IntegralMapType = typename maquis::integral_map<double>;
    static const IntegralMapType integrals;
};

const typename TestSiteproblemFixture::IntegralMapType TestSiteproblemFixture::integrals = maquis::integral_map<double> {
    { { 1, 1, 1, 1 },   0.597263715971     },
    { { 1, 1, 2, 1 },   0.106899493032E-01 },
    { { 2, 1, 2, 1 },   0.215106203079E-02 },
    { { 2, 2, 2, 1 },   0.310109624236E-02 },
    { { 1, 1, 2, 2 },   0.205585394307     },
    { { 2, 2, 2, 2 },   0.266391740477     },
    { { 1, 1, 3, 1 },  -0.294917730728E-08 },
    { { 2, 1, 3, 1 },   0.185509604436E-07 },
    { { 2, 2, 3, 1 },  -0.437276233593E-07 },
    { { 3, 1, 3, 1 },   0.983873746561E-01 },
    { { 3, 2, 3, 1 },  -0.550389961495E-02 },
    { { 3, 3, 3, 1 },  -0.148187533022E-07 },
    { { 1, 1, 3, 2 },   0.656187599279E-07 },
    { { 2, 1, 3, 2 },  -0.686302758417E-08 },
    { { 2, 2, 3, 2 },  -0.185120456521E-07 },
    { { 3, 2, 3, 2 },   0.329840207584E-02 },
    { { 3, 3, 3, 2 },   0.705823689976E-07 },
    { { 1, 1, 3, 3 },   0.505228366944     },
    { { 2, 1, 3, 3 },   0.402391626267E-02 },
    { { 2, 2, 3, 3 },   0.207289323817     },
    { { 3, 3, 3, 3 },   0.485199794875     },
    { { 1, 1, 4, 1 },  -0.179578099756     },
    { { 2, 1, 4, 1 },  -0.968917151241E-02 },
    { { 2, 2, 4, 1 },  -0.985834429751E-02 },
    { { 3, 1, 4, 1 },   0.166302185468E-07 },
    { { 3, 2, 4, 1 },  -0.125974448467E-07 },
    { { 3, 3, 4, 1 },  -0.113847869990     },
    { { 4, 1, 4, 1 },   0.118352897835     },
    { { 4, 2, 4, 1 },   0.102654021605E-01 },
    { { 4, 3, 4, 1 },  -0.130590354090E-07 },
    { { 4, 4, 4, 1 },  -0.121351408757     },
    { { 1, 1, 4, 2 },  -0.350908680238E-01 },
    { { 2, 1, 4, 2 },   0.232966449115E-02 },
    { { 2, 2, 4, 2 },   0.137817149158E-01 },
    { { 3, 1, 4, 2 },   0.112151199425E-07 },
    { { 3, 2, 4, 2 },  -0.240005477894E-07 },
    { { 3, 3, 4, 2 },  -0.312450484337E-01 },
    { { 4, 2, 4, 2 },   0.123070217302E-01 },
    { { 4, 3, 4, 2 },  -0.474523186140E-09 },
    { { 4, 4, 4, 2 },  -0.249154148625E-01 },
    { { 1, 1, 4, 3 },   0.405834174486E-07 },
    { { 2, 1, 4, 3 },   0.416033554153E-08 },
    { { 2, 2, 4, 3 },  -0.294525852258E-07 },
    { { 3, 1, 4, 3 },  -0.444089181829E-02 },
    { { 3, 2, 4, 3 },  -0.612942989364E-02 },
    { { 3, 3, 4, 3 },   0.350197853722E-07 },
    { { 4, 3, 4, 3 },   0.251137992170E-01 },
    { { 4, 4, 4, 3 },   0.323896546368E-07 },
    { { 1, 1, 4, 4 },   0.450421787348     },
    { { 2, 1, 4, 4 },   0.671517333359E-02 },
    { { 2, 2, 4, 4 },   0.195606443342     },
    { { 3, 1, 4, 4 },   0.185362625807E-08 },
    { { 3, 2, 4, 4 },   0.419234994857E-07 },
    { { 3, 3, 4, 4 },   0.382638069339     },
    { { 4, 4, 4, 4 },   0.370380122890     },
    { { 1, 1, 0, 0 },  -0.876082926130     },
    { { 2, 1, 0, 0 },   0.928246209082E-02 },
    { { 2, 2, 0, 0 },  -0.383002645306     },
    { { 3, 1, 0, 0 },   0.110478689971E-07 },
    { { 3, 2, 0, 0 },  -0.838075467983E-07 },
    { { 3, 3, 0, 0 },  -0.192723200850     },
    { { 4, 1, 0, 0 },   0.118275609870     },
    { { 4, 2, 0, 0 },   0.539573313879E-01 },
    { { 4, 3, 0, 0 },  -0.670886115563E-07 },
    { { 4, 4, 0, 0 },  -0.240135259399     },
    { { 0, 0, 0, 0 },   -6.71049529388     }
};

#endif
