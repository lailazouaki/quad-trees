#include <Imagine/Graphics.h>
#include <algorithm>
#include <iostream>
#include "../quadtree/quadtree.h"

using namespace Imagine;
using namespace std;

#ifndef srcPath
#define srcPath(s) ("." s)
#endif

// The path and name of the town file
string default_image_file = srcPath("/running-horse-square.png");

/** Encodage de l'image en QuadTree
 * @brief creer_arbre
 * @param arbre
 * @param x : point de départ de la fenêtre observée (première coordonnée)
 * @param y : point de départ de la fenêtre observée (seconde coordonnée)
 * @param taille : taille de la fenêtre observée
 * @param taille_originale
 * @param image
 * @return
 */
QuadTree<int>* creer_arbre(QuadTree<int> *arbre, int x, int y, int taille, int taille_originale, byte* image){

        bool blanc = false, noir = false;
        for(int i = x ; i < x+taille; i++){
            for(int j = y ; j < y+taille; j++){
                if(image[i+taille_originale*j] == 0) blanc = true;
                if(image[i+taille_originale*j] == 255) noir = true;
            }
        }

        if(blanc && !noir) arbre = new QuadLeaf<int>(0);
        if(!blanc && noir) arbre = new QuadLeaf<int>(255);

        if(blanc &&noir){
            arbre = new QuadNode<int>(0, 0, 0, 0);
            arbre->son(NW) = creer_arbre(arbre->son(NW), x, y, taille/2, taille_originale, image);
            arbre->son(NE) = creer_arbre(arbre->son(NE), x+taille/2, y, taille/2, taille_originale, image);
            arbre->son(SW) = creer_arbre(arbre->son(SW), x, y+taille/2, taille/2, taille_originale, image);
            arbre->son(SE) = creer_arbre(arbre->son(SE), x+taille/2, y+taille/2, taille/2, taille_originale, image);
        }

        return arbre;

}

/** Permet d'obtenir l'image à partir du QuadTree
 * @brief decoder_arbre
 * @param arbre
 * @param x
 * @param y
 * @param taille
 * @param taille_originale
 * @param image
 */

void decoder_arbre(QuadTree<int>* arbre, int x, int y, int taille, int taille_originale, byte *image){
    if(arbre->isLeaf()){
        for(int i = x ; i < x+taille ; i++){
            for(int j = y ; j < y+taille ; j++){
                image[i+taille_originale*j] = arbre->value();
            }
        }
          // Dessine les carrés au fur et à mesure ; à mettre en commentaire pour image_couleur()
       drawRect(x, y, taille, taille, BLACK);
       milliSleep(2);
    }

    else{
        decoder_arbre(arbre->son(NW), x, y, taille/2, taille_originale, image);
        decoder_arbre(arbre->son(NE), x+taille/2, y, taille/2, taille_originale, image);
        decoder_arbre(arbre->son(SW), x, y+taille/2, taille/2, taille_originale, image);
        decoder_arbre(arbre->son(SE), x+taille/2, y+taille/2, taille/2, taille_originale, image);
    }

}

/** Crée explicitement une image carrée à partir d'une image rectangulaire
 * @brief traiter_image_rectangulaire
 * @param image
 * @param image_redimensionnee
 * @param w
 * @param h
 * @param d
 */
void traiter_image_rectangulaire(byte *image, byte* image_redimensionnee, int w, int h, int d){

    for(int i = 0 ; i < w ; i++){
        for(int j = 0 ; j < h ; j++){
            image_redimensionnee[i+d*j] = image[i+w*j];
        }
    }

    for(int k = w ; k < d ; k++){
        for(int l = 0 ; l < h ; l++){
            image_redimensionnee[d*l+k] = 255;
        }
    }

    for(int k = 0 ; k < d ; k++){
        for(int l = h ; l < d ; l++){
            image_redimensionnee[d*l+k] = 255;
        }
    }

}

/** Encodage de l'image en QuadTree pour une image en niveau de gris
 * @brief creer_arbre_gris
 * @param arbre
 * @param x
 * @param y
 * @param taille
 * @param taille_originale
 * @param image
 * @param threshold
 * @return
 */
QuadTree<int>* creer_arbre_gris(QuadTree<int> *arbre, int x, int y, int taille, int taille_originale, byte* image, double threshold){

    int max = image[x+taille_originale*y], min = max;
    for(int i = x ; i < x+taille; i++){
        for(int j = y ; j < y+taille; j++){
            if(image[i+taille_originale*j] > max) max = image[i+taille_originale*j] ;
            if(image[i+taille_originale*j] < min) min = image[i+taille_originale*j];
        }
    }

    if(max - min > threshold){
        arbre = new QuadNode<int>(0, 0, 0, 0);
        arbre->son(NW) = creer_arbre_gris(arbre->son(NW), x, y, taille/2, taille_originale, image, threshold);
        arbre->son(NE) = creer_arbre_gris(arbre->son(NE), x+taille/2, y, taille/2, taille_originale, image, threshold);
        arbre->son(SW) = creer_arbre_gris(arbre->son(SW), x, y+taille/2, taille/2, taille_originale, image, threshold);
        arbre->son(SE) = creer_arbre_gris(arbre->son(SE), x+taille/2, y+taille/2, taille/2, taille_originale, image, threshold);
    }

    else{
        arbre = new QuadLeaf<int>(0.5*(max+min));
    }

    return arbre;
}

/** Affiche l'image obtenue à partir du QuadTree
 * @brief afficher_image
 * @param image
 * @param dimension
 * @param arbre
 */
void afficher_image(byte* image, int dimension, QuadTree<int> *arbre){
    Window window = openWindow(dimension, dimension);
    decoder_arbre(arbre, 0, 0, dimension, dimension, image);
    putGreyImage(IntPoint2(0,0), image, dimension, dimension);
    click();
}

/** Processus complet pour les images en noir et blanc**/
void image_noir_blanc(int argc, char **argv){

        // Get image file
        const char *image_file =
        (argc > 1) ? argv[1] : srcPath("/running-horse-rect.png");
        // Load image
        byte* image;
        int width, height;
        cout << "Loading image: " << image_file << endl;
        loadGreyImage(image_file, image, width, height);
        // Print statistics
        cout << "Image size: " << width << "x" << height << endl;
        cout << "Number of pixels: " << width*height << endl;



        if(width != height){
            int loga = max(log2(width), log2(height));
            int dimension = pow(2, loga+1);

            byte* image_redimensionnee = new byte[dimension*dimension];
            traiter_image_rectangulaire(image, image_redimensionnee, width, height, dimension);

            QuadTree<int>* arbre;
            arbre = creer_arbre(arbre, 0, 0, dimension, dimension, image_redimensionnee);
            afficher_image(image_redimensionnee, dimension, arbre);
            cout<<"Nombre de noeuds : "<<arbre->nbNodes()<<endl;
            double compression = 100*(1-(double)arbre->nbNodes()/(double)(width*height));
            cout<<"Taux de compression : "<<compression<<endl;
        }

        else{
            QuadTree<int>* arbre;
            arbre = creer_arbre(arbre, 0, 0, width, height, image);
            afficher_image(image, width, arbre);
            cout<<arbre->nbNodes()<<endl;
            double compression = 100*(1-(double)arbre->nbNodes()/(double)(width*height));
            cout<<"Taux de compression : "<<compression<<endl;
        }



}

/** Processus complet pour les images en niveau de gris**/
void image_gris(int argc, char **argv){

    // Get image file
    const char *image_file =
    (argc > 1) ? argv[1] : srcPath("/lena.bmp");
    // Load image
    byte* image;
    int width, height;
    cout << "Loading image: " << image_file << endl;
    loadGreyImage(image_file, image, width, height);
    // Print statistics
    cout << "Image size: " << width << "x" << height << endl;
    cout << "Number of pixels: " << width*height << endl;

    QuadTree<int>* arbre;
    arbre = creer_arbre_gris(arbre, 0, 0, width, height, image, 10);
    afficher_image(image, width, arbre);
    double compression = 100*(1-(double)arbre->nbNodes()/(double)(width*height));
    cout<<"Taux de compression : "<<compression<<endl;
}

/** Processus complet pour les images en couleur **/
void image_couleur(int argc, char **argv){
    // Get image file
    const char *image_file =
    (argc > 1) ? argv[1] : srcPath("/lena_couleur.jpg");
    // Load image
    byte* image_r, *image_g, *image_b;
    int width, height;
    cout << "Loading image: " << image_file << endl;
    loadColorImage(image_file, image_r, image_g, image_b, width, height);
    // Print statistics
    cout << "Image size: " << width << "x" << height << endl;
    cout << "Number of pixels: " << width*height << endl;

    QuadTree<int> *arbre_r, *arbre_g, *arbre_b;
    arbre_r = creer_arbre_gris(arbre_r, 0, 0, width, width, image_r, 10);
    arbre_g = creer_arbre_gris(arbre_g, 0, 0, width, width, image_g, 10);
    arbre_b = creer_arbre_gris(arbre_b, 0, 0, width, width, image_b, 10);

    double compression = 100*(1-((double)arbre_r->nbNodes()+(double)arbre_g->nbNodes()+(double)arbre_b->nbNodes())/(double)(3*width*height));
    cout<<"Taux de compression : "<<compression<<endl;

    decoder_arbre(arbre_r, 0, 0, width, width, image_r);
    decoder_arbre(arbre_g, 0, 0, width, width, image_g);
    decoder_arbre(arbre_b, 0, 0, width, width, image_b);

    Window window = openWindow(width, height);
    putColorImage(IntPoint2(0,0), image_r, image_g, image_b, width, width);
    click();
}

int main(int argc, char **argv)
{

      image_noir_blanc(argc, argv);
//      image_gris(argc, argv);
//      image_couleur(argc, argv);

    // Exit
    return 0;
}

