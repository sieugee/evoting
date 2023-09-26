#ifndef TIENNV_ECCELGAMAL
#define TIENNV_ECCELGAMAL

#include <iomanip>
#include <fstream>
#include <cstring>

#include "miracllib/ecn.h"    // Elliptic Curve Class
#include "miracllib/crt.h"         // Chinese Remainder Theorem Class
#include "miracllib/poly.h"
#include "miracllib/polymod.h"

#include "file_name.hpp"
#include "utils.hpp"

using namespace std;

#ifndef MR_NOFULLWIDTH
Miracl precision=10;            // max. 10x32 bits per big number
#else
Miracl precision(10,MAXBASE);
#endif


#if defined(unix)
#define TIMES '.'
#define RAISE '^'
#else
#define TIMES '*'
#define RAISE '#'
#endif

#define ENCODE_FACTOR 614790



/// Classes definitions
class ECCElGamalCipherText{
    public:
        ECn cipher_text_r;
        ECn cipher_text_t;
};

class ECCCurve
{
    public:
        Big a;
        Big b;
        Big p;

        /// Constructor
        ECCCurve(Big a, Big b, Big p)
        {
            if ( !prime(p) )
            {
                cout << "Error, not an elliptic curve\n";
                exit(0);
            }
            this->a = a;
            this->b = b;
            this->p = p;
            ecurve(a, b, p, MR_AFFINE);
        }

        void getCurve()
        {
            cout << "Curve: y^2 = x^3 " << ((a>=0)?" + ":" - ") << a << "x" << ((b>=0)?" + ":" - ") << b << " (mod " << p << ")\n";
        }

        /// @in:      data:         string of data to encode
        //  @in:      len:          data length
        //  @out:     plain_points: pointer to list of points after encode
        //  @returns: 0 if success, -1 if error
        int encode(char* data, size_t len, ECn* plain_points)
        {
            Big xP, yP;
            Big k = Big(ENCODE_FACTOR);

            /// Prepare the list for encode numbers
            size_t arr_size = byteLengthToUInt64Length(len);
            Big number_list[arr_size];
        
            /// Encode process
            uint64_t* dataByWord = reinterpret_cast<uint64_t*>(data);
            for (int i = 0; i < arr_size; i++)
            {
                number_list[i] = Big(*(dataByWord+i));
                Big tmp = k * number_list[i];
                bool found = FALSE;
                for (Big j = Big(0); j < k; j+=1)
                {
                    ECn tmp_pnt = ECn(tmp+j, 2);
                    tmp_pnt.get(xP, yP);
                    if ( xP == tmp+j )
                    {
                        *(plain_points + i) = tmp_pnt;
                        found = TRUE;
                        break;
                    }
                }
                if ( !found )
                {
                    cout << "Failed with number " << tmp << "! You should buy a Vietlott ticket\n";
                    return -1;
                }
        
            }
            return 0;
        }

        //  @in:      plain_points: pointer to list of points to decode
        //  @in:      len:          data length
        /// @out:     data:         string of data after decode
        //  @returns: 0 if success, -1 if error
        char* decode(ECn* points, size_t len, char* data)
        {
            Big xP, yP;
            Big k = Big(ENCODE_FACTOR);

            /// Prepare the list for encode result
            size_t arr_size = byteLengthToUInt64Length(len);
            uint64_t decode_number[arr_size];
            miracl *mip=&precision;

            mip->IOBASE = 16;
            for (int i = 0; i < arr_size; i++)
            {
                char data_str[16];
                (points+i)->get(xP, yP);
                Big tmp = xP/k;
                data_str << tmp;
                decode_number[i] = strtol(data_str, NULL, 16);
            }
            mip->IOBASE = 10;
        
            char* wordByData = reinterpret_cast<char*>(decode_number);
            strcpy(data, wordByData);
            return 0;
        }

};

class ECCElGamal
{
    public:
        /// define the curve
        ECCCurve curve;

        /// Public Key
        ECn public_key_alpha;
        ECn public_key_beta;

    private:
        /// Private Key
        Big private_key_factor;

    public:

        /// Constructor
        ECCElGamal(ECCCurve curve, ECn public_key_alpha, Big private_key_factor)
            : curve(curve)
            , public_key_alpha(public_key_alpha)
            , private_key_factor(private_key_factor)
        {
            ecurve(curve.a, curve.b, curve.p, MR_AFFINE);
            this->public_key_beta = private_key_factor * public_key_alpha;
        }

        ECCElGamalCipherText encrypt(ECn plain_text_point)
        {
            ECn cipher_text_point_1;
            ECn cipher_text_point_2;

            /// Encrypt operation
            ecurve(curve.a, curve.b, curve.p, MR_AFFINE);
            Big k = rand(Big(9999)); /// Should have been curve.p but this is just for demo

            cipher_text_point_1 = k * public_key_alpha;
            cipher_text_point_2 = k * public_key_beta;
            cipher_text_point_2 += plain_text_point;

            ECCElGamalCipherText cipher_text;
            cipher_text.cipher_text_r = cipher_text_point_1;
            cipher_text.cipher_text_t = cipher_text_point_2;
            return cipher_text;
        }

        ECn decrypt(ECCElGamalCipherText cipher_text)
        {
            ECn plain_text_point;

            /// Decrypt operation
            ecurve(curve.a, curve.b, curve.p, MR_AFFINE);

            Big factor = -private_key_factor;

            plain_text_point = factor * cipher_text.cipher_text_r;
            plain_text_point += cipher_text.cipher_text_t;

            return plain_text_point;
        }
};

#endif // TIENNV_ECCELGAMAL
