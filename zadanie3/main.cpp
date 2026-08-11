#include <iostream>
#include  "HybridInspector.h"
int main(){

	std::cout<<"Poczatek lotu:\n";
	HybridInspector inspector;
	inspector.flyToHangar();
	inspector.performIndoorInspection();
	inspector.secureLanding();

	std::cout<<"Koniec lotu\n";

	return 0;
}
