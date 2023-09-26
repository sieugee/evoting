#include "file_utils.hpp"

int main(int argc,char **argv)
{
    int st;
    Big a, b, p, xOne, yOne, xTwo, yTwo;
    ECn ptOne, ptTwo;

    /// Confirm arguments
    if ( argc != 5 )
    {
        return -1;
    }
    
    xOne = Big(argv[1]);
    yOne = Big(argv[2]);
    xTwo = Big(argv[3]);
    yTwo = Big(argv[4]);

    st = readCurveConfig(&a, &b, &p);
    if ( 0 != st )
    {
        return st;
    }
    ECCCurve e(a, b, p);
    e.getCurve();
    
    /// First point
    ptOne = ECn(xOne, yOne);
    ptOne.get(xOne, yOne);
    cout << "First point: x= " << xOne << ", y= " << yOne << "\n";
    if ( ( xOne == 0 ) && ( yOne == 0 ) )
    {
        cout << "Not a point on curve\n";
        return -1;
    }

    /// Second point
    ptTwo = ECn(xTwo, yTwo);
    ptTwo.get(xTwo, yTwo);
    cout << "Second point: x= " << xTwo << ", y= " << yTwo << "\n";
    if ( ( xTwo == 0 ) && ( yTwo == 0 ) )
    {
        cout << "Not a point on curve\n";
        return -1;
    }

    ptOne += ptTwo;

    ptOne.get(xOne, yOne);
    cout << "Result: x= " << xOne << ", y= " << yOne << "\n";

    return 0;
}