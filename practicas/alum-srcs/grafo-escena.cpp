// *********************************************************************
// **
// ** Gestión de una grafo de escena (implementación)
// ** Copyright (C) 2016 Carlos Ureña
// **
// ** This program is free software: you can redistribute it and/or modify
// ** it under the terms of the GNU General Public License as published by
// ** the Free Software Foundation, either version 3 of the License, or
// ** (at your option) any later version.
// **
// ** This program is distributed in the hope that it will be useful,
// ** but WITHOUT ANY WARRANTY; without even the implied warranty of
// ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// ** GNU General Public License for more details.
// **
// ** You should have received a copy of the GNU General Public License
// ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
// **
// *********************************************************************
// ** Informática Gráfica, curso 2018-19
// ** Montserrat Rodríguez Zamorano

#include "aux.hpp"
#include "matrices-tr.hpp"
#include "shaders.hpp"
#include "grafo-escena.hpp"

using namespace std ;

// *********************************************************************
// Entrada del nodo del Grafo de Escena

// ---------------------------------------------------------------------
// Constructor para entrada de tipo sub-objeto

EntradaNGE::EntradaNGE( Objeto3D * pObjeto )
{
   assert( pObjeto != NULL );
   tipo   = TipoEntNGE::objeto ;
   objeto = pObjeto ;
}
// ---------------------------------------------------------------------
// Constructor para entrada de tipo "matriz de transformación"

EntradaNGE::EntradaNGE( const Matriz4f & pMatriz )
{
   tipo    = TipoEntNGE::transformacion ;
   matriz  = new Matriz4f() ; // matriz en el heap, puntero propietario
   *matriz = pMatriz ;
}

// ---------------------------------------------------------------------
// Constructor para entrada de tipo "matriz de transformación"

EntradaNGE::EntradaNGE( Material * pMaterial )
{
   assert( pMaterial != NULL );
   tipo     = TipoEntNGE::material ;
   material = pMaterial ;
}

// -----------------------------------------------------------------------------
// Destructor de una entrada

EntradaNGE::~EntradaNGE(){}

// *****************************************************************************
// Nodo del grafo de escena: contiene una lista de entradas
// *****************************************************************************

// -----------------------------------------------------------------------------
// Visualiza usando OpenGL

void NodoGrafoEscena::visualizarGL( ContextoVis & cv )
{
   glMatrixMode(GL_MODELVIEW); //operamos sobre la modelview
   glPushMatrix() ; //guarda el modelview actual
   cv.pilaMateriales.push();
   for (unsigned i=0; i<entradas.size(); i++){
     if(entradas[i].tipo == TipoEntNGE::objeto){//si la entrada es sub-objeto
       entradas[i].objeto->visualizarGL(cv); //visualizarlo
     }
     else if(entradas[i].tipo == TipoEntNGE::material){
       cv.pilaMateriales.activarMaterial(entradas[i].material);
     }
     else{
       glMatrixMode(GL_MODELVIEW); //modomodelview
       glMultMatrixf(*(entradas[i].matriz)); //componerla
     }
   }
   cv.pilaMateriales.pop();
   glMatrixMode(GL_MODELVIEW); //operamos sobre la modelview
   glPopMatrix(); //restaura modelview guardada
}
// -----------------------------------------------------------------------------

NodoGrafoEscena::NodoGrafoEscena(){}
// -----------------------------------------------------------------------------

void NodoGrafoEscena::fijarColorNodo( const Tupla3f & nuevo_color )
{
  for(int i=0; i<entradas.size();i++){
    if(entradas[i].tipo == TipoEntNGE::objeto){
      entradas[i].objeto->fijarColorNodo(nuevo_color);
    }
  }
}

void NodoGrafoEscena::fijarColorHoja( const Tupla3f & nuevo_color ){
  if(entradas[entradas.size()-1].tipo == TipoEntNGE::objeto){
    entradas[entradas.size()-1].objeto->fijarColorNodo(nuevo_color);
  }
}

// -----------------------------------------------------------------------------
// Añadir una entrada (al final).
// genérica

unsigned NodoGrafoEscena::agregar( const EntradaNGE & entrada )
{
   entradas.push_back(entrada);
   return (entradas.size()-1);
}
// -----------------------------------------------------------------------------
// construir una entrada y añadirla (al final)
// objeto (copia solo puntero)

unsigned NodoGrafoEscena::agregar( Objeto3D * pObjeto )
{
   return agregar( EntradaNGE( pObjeto ) );
}
// ---------------------------------------------------------------------
// construir una entrada y añadirla (al final)
// matriz (copia objeto)

unsigned NodoGrafoEscena::agregar( const Matriz4f & pMatriz )
{
   return agregar( EntradaNGE( pMatriz ) );
}
// ---------------------------------------------------------------------
// material (copia solo puntero)
unsigned NodoGrafoEscena::agregar( Material * pMaterial )
{
   return agregar( EntradaNGE( pMaterial ) );
}

// devuelve el puntero a la matriz en la i-ésima entrada
Matriz4f * NodoGrafoEscena::leerPtrMatriz( unsigned indice ){
  Matriz4f * resultado = entradas.at(indice).matriz;
  return resultado;
}
// -----------------------------------------------------------------------------
// si 'centro_calculado' es 'false', recalcula el centro usando los centros
// de los hijos (el punto medio de la caja englobante de los centros de hijos)

void NodoGrafoEscena::calcularCentroOC()
{

   //   calcular y guardar el centro del nodo
   //    en coordenadas de objeto (hay que hacerlo recursivamente)
   //   (si el centro ya ha sido calculado, no volver a hacerlo)

}
// -----------------------------------------------------------------------------
// método para buscar un objeto con un identificador y devolver un puntero al mismo

bool NodoGrafoEscena::buscarObjeto
(
   const int         ident_busc, // identificador a buscar
   const Matriz4f &  mmodelado,  // matriz de modelado
   Objeto3D       ** objeto,     // (salida) puntero al puntero al objeto
   Tupla3f &         centro_wc   // (salida) centro del objeto en coordenadas del mundo
)
{
  bool salida = false;
  Matriz4f mTMP = mmodelado;
  if(!centro_calculado){
    calcularCentroOC();
  }
   //buscar un sub-objeto con un identificador
   if(leerIdentificador() == ident_busc){
     centro_wc = mmodelado*leerCentroOC();
     *objeto = this;
     salida = true;
   }
   else{ //buscar en los hijos
     bool encontrado = false;
     for(unsigned i=0; i<entradas.size()&&!encontrado; i++){
       if(entradas[i].tipo == TipoEntNGE::objeto){
         encontrado = entradas[i].objeto->buscarObjeto(ident_busc,mTMP,objeto,centro_wc);
       }
       else if(entradas[i].tipo == TipoEntNGE::transformacion){
         mTMP = mTMP*(*entradas[i].matriz);
       }
     }
     salida = encontrado;
   }
   return salida;

}

int NodoGrafoEscena::setIdentificadores(int id){
  ponerIdentificador(id);
  id++;
  for(unsigned i=0; i<entradas.size(); i++){
    if((entradas.at(i)).tipo == TipoEntNGE::objeto){
      (entradas.at(i)).objeto -> ponerIdentificador(id);
      id++;
    }
  }
  return id;
}

// *****************************************************************************
// Nodo del grafo de escena, con una lista añadida de parámetros
// *****************************************************************************


// -----------------------------------------------------------------------------
// devuelve el numero de grados de libertad
int NodoGrafoEscenaParam::numParametros(){
  return parametros.size();
}
// -----------------------------------------------------------------------------

// devuelve un puntero al i-ésimo grado de libertad
Parametro * NodoGrafoEscenaParam::leerPtrParametro( unsigned i ){
  return &parametros[i];
}
// -----------------------------------------------------------------------------

void NodoGrafoEscenaParam::siguienteCuadro(){
  for(unsigned i=0; i<parametros.size();i++){
    (parametros[i]).siguiente_cuadro();
  }
}

////////////////////////////////////////////////////////////////////////////
Pelota::Pelota(vector <Parametro> *p){
  ponerNombre("pelota");

  agregar(MAT_Traslacion(0.0,0.0,0.0));
  agregar(MAT_Traslacion(-2.5,1.0,0.0));
  agregar(MAT_Escalado(0.5,0.5,0.5));
  Esfera * esf = new Esfera(1000,1000,false,false);
  agregar(new MaterialPelota());
  agregar(esf);
  string mensaje = "Movimiento de la pelota: Botar.";
  p->push_back(Parametro(mensaje, entradas[0].matriz,
              [=](float v){return MAT_Traslacion(0.0,v,0.0);},
              true, 0.0, 0.5, 0.1));
  fijarColorNodo(Tupla3f(0.5,0.18,0.18));
}

Base::Base(vector <Parametro> *p){
  ponerNombre("base lámpara");

  agregar(MAT_Escalado(1.5,0.5,1.5));
  Cilindro * cil = new Cilindro(5,1000,true,true);
  agregar(new MaterialFlexo());
  agregar(cil);
  fijarColorNodo(Tupla3f(0.5,0.5,0.5));
}

Barra::Barra(vector <Parametro> *p){
  ponerNombre("barra flexo");

  agregar(MAT_Traslacion(0.0,0.4,0.0));
  agregar(MAT_Escalado(0.2,3.0,0.2));
  Cilindro * cil = new Cilindro(5,1000,true,true);
  agregar(new MaterialFlexo());
  agregar(cil);
  fijarColorNodo(Tupla3f(0.5,0.5,0.5));
}

LamparaSuperior::LamparaSuperior(vector <Parametro> *p){
  ponerNombre("lampara superior");

  agregar(MAT_Rotacion(0.0,0.0,1.0,0.0)); //esta matriz la modificara el parametro 2
  agregar(MAT_Rotacion(0.0,0.0,1.0,0.0)); //esta matriz la modificara el parametro 0
  agregar(MAT_Traslacion(0.0,3.25,0.0));
  agregar(MAT_Rotacion(0.0,0.0,1.0,0.0)); //esta matriz la modificara el parametro 1
  //en este punto la lampara sup esta formada y centrada
  agregar(MAT_Traslacion(-0.9,0.0,0.0));
  agregar(MAT_Rotacion(-90.0,0.0,0.0,1.0));
  agregar(MAT_Escalado(0.5,0.5,0.5));
  Esfera * esf = new Esfera(1000,1000,true,true);
  agregar(new MaterialBombilla());
  agregar(esf);
  fijarColorHoja(Tupla3f(1.0,0.8,0.0));
  agregar(MAT_Escalado(2.0,2.0,2.0));
  agregar(MAT_Traslacion(0.0,-0.5,0.0));
  ConoTruncado * ct = new ConoTruncado(1.0,0.5,5,1000,false,true);
  agregar(new MaterialFlexo());
  agregar(ct);
  fijarColorHoja(Tupla3f(0.5,0.5,0.5));
  agregar(MAT_Traslacion(0.0,1.0,0.0));
  agregar(MAT_Escalado(0.5,1.0,0.5));
  Cilindro * cil = new Cilindro(5,1000,true,true);
  agregar(new MaterialFlexo());
  agregar(cil);
  fijarColorHoja(Tupla3f(0.5,0.5,0.5));
  //agregamos el parámetro asociado
  string mensaje = "Rotación a los lados del cabezal del flexo.";
  p->push_back(Parametro(mensaje, entradas[1].matriz,
              [=](float v){return MAT_Rotacion(v,0.0,1.0,0.0);},
              false, 0.0, 15.0, 0.5));
  string mensaje2 = "Rotación arriba-abajo del cabezal del flexo.";
  p->push_back(Parametro(mensaje2, entradas[3].matriz,
              [=](float v){return MAT_Rotacion(v,0.0,0.0,1.0);},
              true, 0.0, 20.0, 0.2));
  string mensaje3 = "Desplazamiento lateral del cabezal del flexo.";
  p->push_back(Parametro(mensaje3, entradas[0].matriz,
                [=](float v){return MAT_Traslacion(v,0.0,0.0);},
                true, 0.0, 0.15, 0.03));
}

Lampara::Lampara(){
  Base *base = new Base(&parametros);
  Barra *barra = new Barra(&parametros);
  LamparaSuperior *ls = new LamparaSuperior(&parametros);
  Pelota *pelota = new Pelota(&parametros);
  agregar(pelota);
  agregar(ls);
  agregar(barra);
  agregar(base);

  ponerIdentificador(50);
}

void Lampara::reiniciar(){
  for(unsigned i=0; i<parametros.size(); i++){
    parametros.at(i).reset();
  }
}

/////////////////////////// PRACTICA 4 /////////////////////////////////

Lata::Lata(){
  ponerNombre("Lata");
  Objeto3D * lataInf = new MallaRevol("../plys/lata-pinf.ply",100,true,true,true);
  Objeto3D * lataCue = new MallaRevol("../plys/lata-pcue.ply",100,false,true,true);
  Objeto3D * lataSup = new MallaRevol("../plys/lata-psup.ply",100,true,true,true);

  agregar(new MaterialTapasLata());
  agregar(lataSup);
  agregar(lataInf);
  agregar(new MaterialLata());
  agregar(lataCue);

  ponerIdentificador(40);

}

PeonBlanco::PeonBlanco(){
  ponerNombre("Peon blanco");

  Objeto3D *peon = new MallaRevol("../plys/peon.ply",1000,true,true,false);

  agregar(MAT_Traslacion(-0.5,0.28,0.7));
  agregar(MAT_Escalado(0.2,0.2,0.2));
  agregar(new MaterialPeonBlanco());
  agregar(peon);
  ponerIdentificador(30);
}

PeonMadera::PeonMadera(){
  ponerNombre("Peon madera");

  Objeto3D *peon = new MallaRevol("../plys/peon.ply",1000,true,true,false);

  agregar(MAT_Traslacion(0.0,0.28,0.7));
  agregar(MAT_Escalado(0.2,0.2,0.2));
  agregar(new MaterialPeonMadera());
  agregar(peon);
  ponerIdentificador(20);
}

PeonNegro::PeonNegro(){
  ponerNombre("Peon negro");
  Objeto3D *peon = new MallaRevol("../plys/peon.ply",1000,true,true,false);

  agregar(MAT_Traslacion(0.5,0.28,0.7));
  agregar(MAT_Escalado(0.2,0.2,0.2));
  agregar(new MaterialPeonNegro());
  agregar(peon);
  ponerIdentificador(10);
}

EscenaObjetosLuces::EscenaObjetosLuces(){
  Lata * l = new Lata();
  //l->ponerIdentificador(40);
  PeonBlanco * pb = new PeonBlanco();
  //pb->ponerIdentificador(30);
  PeonMadera * pm = new PeonMadera();
  //pm->ponerIdentificador(20);
  PeonNegro *pn = new PeonNegro();
  //pn->ponerIdentificador(10);

  agregar(l);
  agregar(pb);
  agregar(pn);
  agregar(pm);
}
