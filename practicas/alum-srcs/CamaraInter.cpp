#include <aux.hpp>
#include "CamaraInter.hpp"


// -----------------------------------------------------------------------------
// constructor de cámaras interactivas, los parámetros son:
//
//    * examinar_ini: fija modo examinar (true) o modo primera persona (false)
//    * ratio_yx_vp_ini: ratio del viewport (alto/ancho)
//    * longi_ini_grad, lati_ini_grad: longitud y latitud iniciales (en grados)
//    * aten_ini: punto de atención inicial
//    * pers_ini: fija proy. perspectiva (true) u ortográfica (false)
//
// ** Informática Gráfica, curso 2018-19
// ** Montserrat Rodríguez Zamorano

CamaraInteractiva::CamaraInteractiva( bool examinar_ini, float ratio_yx_vp_ini,
                                      float longi_ini_grad, float lati_ini_grad,
                                      const Tupla3f & aten_ini, bool pers_ini,
                                      float hfov_grad_ini, float dist_ini  )
{
   using namespace std ;

   examinar    = examinar_ini ;
   longi       = longi_ini_grad ;
   lati        = lati_ini_grad ;
   dist        = dist_ini ;
   aten        = aten_ini ;
   ratio_yx_vp = ratio_yx_vp_ini; //cout << "ratio_yx_vp_ini == " << ratio_yx_vp_ini << endl ;
   perspectiva = pers_ini ;
   hfov_grad   = hfov_grad_ini ;
   dx          = 0.0 ;
   dy          = 0.0 ;

   calcularViewfrustum(  );  // lee: dist, ratio_yx_vp, perspectiva
   calcularMarcoCamara();    // lee: longi, lati, dist, aten
}


// ----------------------------------------------------------------------------
// calcula view-frustum (la matriz de proyección) usando:
//    perspectiva, dist, ratio_yx_vp

constexpr float n = 0.01 ; // valor 'near' en proy. persp


void CamaraInteractiva::calcularViewfrustum(){
   using namespace std ;
   float far = dist+500; //500 por ejemplo
   float aspect = 1.0/ratio_yx_vp; //lo invertimos porque aspect corresponde a la relacion ancho/alto
   if (perspectiva){ //caso camara perspectiva
     vf = ViewFrustum(hfov_grad, aspect, n, far);
   }
   else{ //caso camara ortografica: usar ratio_yx_vp, dist, función MAT_Ortografica
    // Calcular los parámetros del view frustum (vf)
    //actualiza la matriz de proyección (vf.matrizProy)
      vf.persp=false;
      vf.left=-dist*aspect;
      vf.right=dist*aspect;
      vf.bottom=-dist;
      vf.top=dist;
      vf.near=n;
      vf.far=far;
      vf.matrizProy=MAT_Ortografica(vf.left, vf.right, vf.bottom, vf.top, vf.near, vf.far);
   }

}

//-----------------------------------------------------------------------
// calcula el marco de referencia de la camara (y la matriz de vista), usando:
//     longi, lati, dist, aten

void CamaraInteractiva::calcularMarcoCamara()
{
   // Recalcular 'mcv.matrizVista' y 'mcv.matriVistaInv' usando la funcion
   // (1) Matriz = Trasl( aten )*Rotacion( longi, Y )*Rotacion( -lati, X )* Traslacion( (0,0,dist) )
   Matriz4f matriz = MAT_Traslacion(aten)*
                    MAT_Rotacion(longi,0.0,1.0,0.0)*
                    MAT_Rotacion(lati,1.0,0.0,0.0)*
                    MAT_Traslacion(0.0,0.0,dist);
  //    (2) ejes mcv = ejes mcv * matriz
    mcv.eje[0] = matriz*Tupla4f(1.0, 0.0, 0.0, 0.0);
    mcv.eje[1] = matriz*Tupla4f(0.0, 1.0, 0.0, 0.0);
    mcv.eje[2] = matriz*Tupla4f(0.0, 0.0, 1.0, 0.0);
    mcv.eje[3] = matriz*Tupla4f(0.0, 0.0, 0.0, 1.0);
  //    (3) recalcular matrices marco camara
    recalcularMatrMCV();
}


//-----------------------------------------------------------------------
// actualiza las matrices que hay en 'cam.mcv'
// a patir de 'cam.mcv.org' y 'cam.mcv.ejes'

void CamaraInteractiva::recalcularMatrMCV()
{
   // Recalcular 'mcv.matrizVista' y 'mcv.matriVistaInv' en 'mcv' usando 'mcv.eje[X/Y/Z]' y 'mcv.org'
   mcv.matrizVista = MAT_Vista(mcv.eje, mcv.org);
   mcv.matrizVistaInv = MAT_Vista_inv(mcv.eje, mcv.org);
}


// -----------------------------------------------------------------------------
//  métodos para manipular (desplazar o rotar) la cámara
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// desplazar o rotar la cámara en horizontal/vertical

constexpr float urot  = 1,    // unidad de rotación (1 grado)
                udesp = 0.01 ;  // unidad de desplazamiento

void CamaraInteractiva::moverHV( float nh, float nv )
{
   using namespace std ;
   if ( examinar ) // modo examinar
   {
      // Actualizar 'longi' y 'lati' y recalcular marco de cámara
      longi += nh*urot;
      lati += nv*urot;
      calcularMarcoCamara();

   }
   else // modo primera persona
   {
      // Desplazar 'mcv.org' en X e Y y recalcular la matriz de vista
      mcv.org[0]+= nh*udesp;
      mcv.org[1]+= nv*udesp;
      recalcularMatrMCV();
      // (movimiento solidario del punto de atención)
      aten[0]+= nh*udesp;
      aten[1]+= nv*udesp;
   }
}
// -----------------------------------------------------------------------------
// desplazar en el eje Z de la cámara (hacia adelante o hacia detrás)

constexpr float dmin = 2.0*n,  // distancia minima (2*near)
                porc = 2 ; // porcentaje de cambio (1%)

//desplazamiento en la direccion en la que el usuario esta mirando la camara
void CamaraInteractiva::desplaZ( float nz )
{
   using namespace std ;

   if ( examinar ) // examinar
   {
      // Actualizar 'dist' usando 'nz', 'dmin' y 'porc' y recalcular marco de cámara
      dist = dmin + (dist-dmin)*(1.0-nz*porc/100.0);
      calcularMarcoCamara();
   }
   else // primer persona
   {
      // Desplazar 'mcv.org' en Z, y recalcular la matriz de vista
      // - en vez de + porque tiene que ser en la direccion en la que el usuario mira
      mcv.org[2]-=nz*udesp;
      recalcularMatrMCV();
      // (y movimiento solidario del punto de atención)
      aten[2]-=nz*udesp;
   }

}
// -----------------------------------------------------------------------------



// -----------------------------------------------------------------------------
// métodos para cambiar de modo
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// fija punt. aten. y activa modo examinar
void CamaraInteractiva::modoExaminar( const Tupla3f & pAten )
{
   // Fija punt. aten. y activa modo examinar, recalcula marco de camara
   aten = pAten;
   examinar = true;
   calcularMarcoCamara();

}
// -----------------------------------------------------------------------------
// pasa a modo examinar (mantiene p.aten.)

void CamaraInteractiva::modoExaminar()
{
   // Activa modo examinar (no es necesario recalcular el marco de cámara)
   examinar = true;

}
// -----------------------------------------------------------------------------
// pasa al modo primera persona

void CamaraInteractiva::modoPrimeraPersona()
{
   // Activa modo primera persona (no es necesario recalcular el marco de cámara)
   examinar = false;
}
