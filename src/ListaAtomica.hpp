#ifndef LISTA_ATOMICA_H__
#define LISTA_ATOMICA_H__

#include <atomic>
#include <pthread.h>

template <typename T>
class Lista {
private:
	struct Nodo {
		Nodo(const T& val) : _val(val), _next(nullptr) {}
		T _val;
		Nodo *_next;
	};

	std::atomic<Nodo *> _head;
	pthread_mutex_t mutex;
public:
	Lista() : _head(nullptr) {
		pthread_mutex_init(&mutex, NULL);

		}
	~Lista() {
		Nodo *n, *t;
		n = _head.load();
		while (n) {
			t = n;
			n = n->_next;
			delete t;
		}
		pthread_mutex_destroy(&mutex);
	}

	void push_front(const T& val) {
		/* Completar. Debe ser atómico. */
		//PREGUNTAR A QUE SE REFIEREN CON ATOMICO
		Nodo* nodillo = new Nodo(val);
		pthread_mutex_lock(&mutex);
		nodillo->_next=_head;
		_head=nodillo;
		pthread_mutex_unlock(&mutex);
	}

	T& front() const {
		return _head.load()->_val;
	}

	T& iesimo(int i) const {
		Nodo *n = _head.load();
		int j;
		for (int j = 0; j < i; j++)
			n = n->_next;
		return n->_val;
	}

	class Iterador {
	public:
		Iterador() : _lista(nullptr), _nodo_sig(nullptr) {}

		Iterador& operator = (const typename Lista::Iterador& otro) {
			_lista = otro._lista;
			_nodo_sig = otro._nodo_sig;
			return *this;
		}

		bool HaySiguiente() const {
			return _nodo_sig != nullptr;
		}

		T& Siguiente() {
			return _nodo_sig->_val;
		}

		void Avanzar() {
			_nodo_sig = _nodo_sig->_next;
		}

		bool operator == (const typename Lista::Iterador& otro) const {
			return _lista._head.load() == otro._lista._head.load() && _nodo_sig == otro._nodo_sig;
		}

	private:
		Lista *_lista;

		typename Lista::Nodo *_nodo_sig;

		Iterador(Lista<T>* lista,typename Lista<T>::Nodo* sig) : _lista(lista), _nodo_sig(sig) {}
		friend typename Lista<T>::Iterador Lista<T>::CrearIt();
	};

	Iterador CrearIt() {
		return Iterador(this, _head);
	}
};

#endif /* LISTA_ATOMICA_H__ */
