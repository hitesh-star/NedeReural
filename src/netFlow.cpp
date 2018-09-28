#include "netFlow.h"
#include "debug.h"
#include "logger.h"
#include "iomanip"

const unsigned int NUM_PIXELS = 784;
const unsigned int SIZE_HIDDEN = 16;
const std::string DATA_PATH = "./data/";
const std::string INPUT_PATH = "./input/image.txt";
const std::string TRAIN_IMG_PATH = "./data/images-1.ubyte";
const std::string TRAIN_LABEL_PATH = "./data/labels-1.ubyte";
const std::string TEST_IMG_PATH = "./data/images-2.ubyte";
const std::string TEST_LABEL_PATH = "./data/labels-2.ubyte";
const int BATCH_SIZE = 15;
const int TRAIN_SIZE = 1200;
const int EPOCH_AMOUNT = 5; // EPOCH_AMOUNT * BATCH_SIZE <= 60000

bool drawing;
cv::Mat img;
const double LEARN_RATE = 6.0;


Network network;

bool existsFile(std::string path);
void initializeNetwork(InputReader);
Data createSampleData(Network tmp);
void printEpoch(int epoch, int correct, int total);
std::vector<TrainingExample> computeReducedTrain(std::vector<TrainingExample> a);

std::string execute() {

    InputReader reader;
    Output output;

    log("Inicializando a network");
    initializeNetwork(reader);

    /* Ler arquivo */
    if (!existsFile(INPUT_PATH)) {
        throw "No file " + INPUT_PATH;
    }

    // Criar matriz coluna que representa as ativacoes
    // de entrada da rede neural
    log("Lendo imagem padrao");
    Narray image = desenhar();

    log("Iniciando o feedfoward");
    network.feedfoward(image);

    log("Iniciando print de resposta");
    std::string answer = output.print(network.output.value);
    image.close();
    network.close();
    return answer;
}

void train(){
    
    InputReader reader;
    Output output;

    log("Inicializando a network");
    initializeNetwork(reader);

    std::vector<TrainingExample> trainSet;
    std::vector<TrainingExample> testSet;
    std::vector<TrainingExample> reducedTrainSet;


    /* Computar os conjuntos de treino e teste */
    //while(true);
    log("Lendo o trainSet");
    trainSet = reader.binaryTrainings(TRAIN_IMG_PATH, TRAIN_LABEL_PATH, 1);
    log("Lendo o testSet");
    testSet = reader.binaryTrainings(TEST_IMG_PATH, TEST_LABEL_PATH, 1);
    /* Executar e testar epocas de treino */
    log("Iniciando execucao das training epochs");
        log("Randomizando o trainSet");
        std::random_shuffle(trainSet.begin(), trainSet.end());

        log("Randomizando o testSet");
        std::random_shuffle(testSet.begin(), testSet.end());

        log("Gerando trainSet reduzido");
        reducedTrainSet = computeReducedTrain(trainSet);
        


        int batchAmount = reducedTrainSet.size() / BATCH_SIZE;
    for (int i = 0; i < EPOCH_AMOUNT; i++) {

        log("Inicializando uma training epoch");
        network.trainingEpoch(reducedTrainSet, BATCH_SIZE, batchAmount, LEARN_RATE);

        log("Inicializando teste de epoch");
        int correctCnt = network.testEpoch(testSet);

        log("Printando qualidade da epoch");
        printEpoch(i, correctCnt, BATCH_SIZE * batchAmount);

        log("Salvando as informacoes da rede");
        save(reader);
    }
    network.close();
}


void printEpoch(int epoch, int correct, int total) {
    double quality = correct * 100.0 / total;
    std::cout << "EPOCH #" << epoch << ": " << std::fixed 
    << std::setprecision(2) << quality << "\%" <<
    " ~ " << correct << "/" << total << std::endl;
}

// NAO MEXER, GAMBIARRA E DESESPERO ABAIXO
void save(InputReader ir){
    Data info = Data(network.hidden.weight, network.output.weight,
                     network.hidden.bias, network.output.bias);
    // ir.clear(DATA_PATH + "weightsHidden.txt");
    ir.fillArchive(DATA_PATH + "weightsHidden.txt", info.weightsHidden);

    // ir.clear(DATA_PATH + "weightsOutput.txt");
    ir.fillArchive(DATA_PATH + "weightsOutput.txt", info.weightsHidden);

    // ir.clear(DATA_PATH + "biasesHidden.txt");
    ir.fillArchive(DATA_PATH + "biasesHidden.txt", info.weightsHidden);

    // ir.clear(DATA_PATH + "biasesOutput.txt");
    ir.fillArchive(DATA_PATH + "biasesOutput.txt", info.weightsHidden);
}

std::vector<TrainingExample> computeReducedTrain(std::vector<TrainingExample> a){
    std::vector<TrainingExample> reducedTrainSet;

    for (int i = 0; i < TRAIN_SIZE; i++){
        reducedTrainSet.push_back(a[i]);
    }

    return reducedTrainSet;
}

bool existsFile(std::string path) {
    InputReader ir;
    return !ir.empty(path);
}

void initializeNetwork(InputReader reader){
    /* Checar casos de execucao */
    bool isFirstExec = !existsFile(DATA_PATH);
    if (true) {
        network = Network(NUM_PIXELS, SIZE_HIDDEN);
    } else {
        Network tmp = Network();
        Data info = createSampleData(tmp);

        log("Lendo informacoes de treino da rede");
        info.weightsHidden = reader.readMatrix(DATA_PATH + "weightsHidden.txt", 
                             info.weightsHidden.row, info.weightsHidden.colunm);
        info.weightsOutput = reader.readMatrix(DATA_PATH + "weightsOutput.txt", 
                             info.weightsOutput.row, info.weightsOutput.colunm);
        info.biasesOutput = reader.readMatrix(DATA_PATH + "biasesOutput.txt", 
                             info.biasesOutput.row, info.biasesOutput.colunm);
        info.biasesHidden = reader.readMatrix(DATA_PATH + "biasesHidden.txt", 
                             info.biasesHidden.row, info.biasesHidden.colunm);

        log("Criando rede com informacoes customizadas de treino");
        network = Network(NUM_PIXELS, SIZE_HIDDEN, info);
    }
}

Data createSampleData(Network tmp){
    return Data(tmp.hidden.weight, tmp.output.weight,
                tmp.hidden.bias, tmp.output.bias);
}

Narray desenhar(){
    drawing = false;
    img = cv::Mat(28, 28, CV_8UC3, cv::Scalar(0,0,0));
    cv::namedWindow("Desenhe");
    cv::setMouseCallback("Desenhe", draw_circle);
    while(1){
        cv::imshow("Desenhe",img);
        int k = cv::waitKey(1) & 0xFF;
        if (k == 's'){
            Narray ret(28 * 28, 1);
            for(int i = 0; i < 28; i++){
                for(int j = 0; j < 28; j++){
                    ret.values[i * 28 + j][0] = img.at<uchar>(cv::Point(i, j)) / 255.0;
                }
            }
            cv::destroyAllWindows();
            return ret;
        }
    }
}

void draw_circle(int event, int x, int y, int flags, void* param){
    if (event == cv::EVENT_LBUTTONDOWN){
        drawing = true;
    }else if(event == cv::EVENT_MOUSEMOVE){
        if (drawing == true)
            cv::circle(img,cv::Point(x,y),1,cv::Scalar(255,255,255),-1, cv::LINE_AA);
    }else if(event == cv::EVENT_LBUTTONUP){
        drawing = false;
        cv::circle(img,cv::Point(x,y),1,cv::Scalar(255,255,255),-1, cv::LINE_AA);
    }
}
