#include "file_utils.hpp"

int main(int argc,char **argv)
{
    int st;
    Big a, b, p, xP, yP, factor;
    ECn P, result;

    /// Confirm arguments
    if ( ( argc > 4 ) && ( argc < 3 ) )
    {
        return -1;
    }
    else
    {
        factor = Big(argv[1]);
        xP = Big(argv[2]);
        if ( argc == 4 )
        {
            yP = Big(argv[3]);
        }
    }

    st = readCurveConfig(&a, &b, &p);
    if ( 0 != st )
    {
        return st;
    }

    /// Multiply
    ECCCurve e(a, b, p);
    e.getCurve();
    if ( argc == 4 ) {
        P = ECn(xP, yP);
    }
    else /// argc == 3
    {
        P = ECn(xP, 1);
    }
    P.get(xP, yP);

    cout << "Original point: x= " << xP << ", y= " << yP << "\n";
    cout << "Factor: " << factor << "\n";

    if ( ( xP == 0 ) && ( yP == 0 ) )
    {
        cout << "Not a point on curve\n";
        return -1;
    }


    result = factor * P;

    result.get(xP, yP);
    cout << "Result: x= " << xP << ", y= " << yP << "\n";

    return 0;
}