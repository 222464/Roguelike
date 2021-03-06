#pragma once

#include "RSDR.h"

namespace sdr {
	class PredictiveRSDR {
	public:
		struct Connection {
			unsigned short _index;

			float _weight;
		};

		struct LayerDesc {
			int _width, _height;

			int _receptiveRadius, _recurrentRadius, _lateralRadius, _predictiveRadius, _feedBackRadius;

			float _learnFeedForward, _learnRecurrent, _learnLateral, _learnBias;

			float _learnFeedBack, _learnPrediction;

			float _averageSurpriseDecay;
			float _attentionFactor;

			float _sparsity;

			LayerDesc()
				: _width(16), _height(16),
				_receptiveRadius(10), _recurrentRadius(6), _lateralRadius(5), _predictiveRadius(7), _feedBackRadius(8),
				_learnFeedForward(0.003f), _learnRecurrent(0.003f), _learnLateral(0.03f), _learnBias(0.005f),
				_learnFeedBack(0.01f), _learnPrediction(0.01f),
				_averageSurpriseDecay(0.01f),
				_attentionFactor(4.0f),
				_sparsity(0.04f)
			{}
		};

		struct PredictionNode {
			std::vector<Connection> _feedBackConnections;
			std::vector<Connection> _predictiveConnections;

			Connection _bias;

			float _state;
			float _statePrev;

			float _activation;
			float _activationPrev;

			float _averageSurprise; // Use to keep track of importance for prediction. If current error is greater than average, then attention is > 0.5 else < 0.5 (sigmoid)

			PredictionNode()
				: _state(0.0f), _statePrev(0.0f), _activation(0.0f), _activationPrev(0.0f), _averageSurprise(0.0f)
			{}
		};

		struct Layer {
			RSDR _sdr;

			std::vector<PredictionNode> _predictionNodes;
		};

		static float sigmoid(float x) {
			return 1.0f / (1.0f + std::exp(-x));
		}
	
	private:
		std::vector<LayerDesc> _layerDescs;
		std::vector<Layer> _layers;

		std::vector<float> _prediction;

	public:
		void createRandom(int inputWidth, int inputHeight, const std::vector<LayerDesc> &layerDescs, float initMinWeight, float initMaxWeight, float initMinInhibition, float initMaxInhibition, std::mt19937 &generator);

		void simStep();

		void setInput(int index, float value) {
			_layers.front()._sdr.setVisibleInput(index, value);
		}

		void setInput(int x, int y, float value) {
			setInput(x + y * _layers.front()._sdr.getVisibleWidth(), value);
		}

		float getPrediction(int index) const {
			return _prediction[index];
		}

		float getPrediction(int x, int y) const {
			return getPrediction(x + y * _layers.front()._sdr.getVisibleWidth());
		}

		const std::vector<LayerDesc> &getLayerDescs() const {
			return _layerDescs;
		}

		const std::vector<Layer> &getLayers() const {
			return _layers;
		}
	};
}