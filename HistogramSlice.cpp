// File name: HistogramSlice.cpp
// Author: Viet Than
// Email: viet.than@vanderbilt.edu (thanhoangviet@gmail.com)
// Lab: Medical Imaging Lab under Ipek Oguz
// Description: Histogram normalize a volume slice by slice with reference to the slice middle,
// 		output differently depend on direction.



#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkExtractImageFilter.h"


#include "itkHistogramMatchingImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

#include <string>
#include <iostream>
#include <chrono>
#include <iomanip> //set precision
#include <sstream> //stringstream

using namespace itk;

//helper functions
std::string makeInputFileName (const std::string &filename, const std::string &filetype);
std::string makeOutputFileName (const std::string &filename, const std::string &filetype);


template <typename T> std::string returnPointString(const T &number);
void extract2DNormal (const std::string &filename, const std::string &filetype);

// 5 arguments:
// 1 - filename
// 2 - intputType
// 3 - outputType
// 4 - orientation x:0, y=1, z=2
// 5 - scaleToUsual
int main(int argc, char * argv []){

	std::cout << "Starting histogram filter on slices"  << std::endl;

	if (argc > 5){
		std::cout << "too many arguments" << std::endl;
		return EXIT_FAILURE;
	}

	// setting up arguments
	std::string filename, inputType, outputType;
	int orientation, scaleToUsual;
	
	// constexpr, computation at compile time
	constexpr unsigned int Dimension = 3;
	constexpr float intensityMinimum = 0.0;
	constexpr float intensityMaximum = 255.0;
	
	if (argc == 6){
		std::cout << "Accepted input arguments" << std::endl;
		filename = argv[1];
		inputType = argv[2];
		outputType = argv[3];
		orientation = argv[4];
		scaleToUsual = argv[5];
	} else {
		std::cout << "Not enough arguments, went with default" << std::endl;
		filename = "Smallfield_OCT_Angiography_Volume_fovea"; //filename in data/
		inputType = ".nii";
		outputType = inputType;
		orientation = 0;
		scaleToUsual = 0;

	}
	//timing
	auto start = std::chrono::high_resolution_clock::now();	

	std::string inputFileName = makeInputFileName(filename, inputType);
	std::string outputFileName = makeOutputFileName(filename, outputType);
	
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << duration.count() << " milliseconds for reading in the file and creating constants"<<std::endl;
	
	// setting up reader type
	using imagePixelType = short;
	using ImageType = itk::Image< InputPixelType, 3>;
	std::string inputFileName = makeInputFileName(filename, filetype);
	using ReaderType = itk::ImageFileReader< ImageType >;
	
	// Setting up writer
	using WriterType = itk::ImageFileWriter< ImageType >;
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName( outputFileName );

	/*THIS IS ALL TO EXTRACT MIDDLE SLICE*/
	
	// setting up middle slice reader
	ReaderType::Pointer middleReader = ReaderType::New();
	middleReader->SetFileName( inputFileName );
	
	// update middle slice reader with filename
  	try{
    		middleReader->Update();
	} catch( itk::ExceptionObject & err ){
    		std::cerr << "ExceptionObject caught !" << std::endl;
    		std::cerr << err << std::endl;
    		return EXIT_FAILURE;
    	}

	// getting the middle slice from middle slice reader
	const ImageType * inputImage = middleReader->GetOutput(); // get the input image
	ImageType::RegionType inputRegion = inputImage->GetLargestPossibleRegion(); // get image region
	ImageType::SizeType size = inputRegion.GetSize(); //getting the region size
	ImageType::IndexType middleStart = inputRegion.GetIndex(); // get index
	const unsigned int midSliceNumber = (int) size[orientation]/2; // finding middle slice
	ImageType::SizeType middleSize = size; // set up the size for middle slice
	middleSize[orientation] = 1; // setting the orientation direction to 1 for collapse for middle slice size

	// set up extraction region for middle slice
	ImageType::RegionType desiredMiddleRegion; // create middle region
	desiredMiddleRegion.SetSize( middleSize ); // set middle region to size (with collapsed direction)
	desiredMiddleRegion.SetIndex( middleStart ); // set extraction region index start

	// setting up Extraction filter for middle slice
	using ExtractFilterType = itk::ExtractImageFilter< ImageType, ImageType >; //
	ExtractFilterType::Pointer extractMiddleFilter = ExtractFilterType::New();
	extractMiddleFilter->SetDirectionCollapseToSubmatrix();
	extractMiddleFilter->SetExtractionRegion( desiredMiddleRegion );
	extractFilter->SetInput( inputImage );


	/*THIS IS ALL FOR EVERY OTHER SLICE*/

	










	using InputPixelType = float;
	using OutputPixelType = float;

	using InputImageType = itk::Image< InputPixelType, 3 >;
	using OutputImageType = itk::Image< OutputPixelType, 2>;

	using ReaderType = itk::ImageFileReader< InputImageType >;
	using WriterType = itk::ImageFileWriter< OutputImageType >;

	std::string outputName = "../output/";
	outputName.append(filename);
	outputName.append("_ChosenMiddleSlice").append(".tif");

	ReaderType::Pointer reader = ReaderType::New();
	WriterType::Pointer writer = WriterType::New();


	reader->SetFileName( inputFileName );
	writer->SetFileName( outputName );
	reader->Update();

	using FilterType = itk::ExtractImageFilter< InputImageType, OutputImageType > ;
	FilterType::Pointer filter = FilterType::New();
	filter->InPlaceOn();
	filter->SetDirectionCollapseToSubmatrix();

	InputImageType::RegionType inputRegion = reader->GetOutput()->GetLargestPossibleRegion();

	InputImageType::SizeType size = inputRegion.GetSize();
	size[0] = 0;
	std::cout << size << std::endl;

	InputImageType::IndexType start = inputRegion.GetIndex();
	const unsigned int sliceNumber = 250;
	start[0] = sliceNumber;
	std::cout << start << std::endl;

	InputImageType::RegionType desiredRegion;
	desiredRegion.SetSize( size );
	desiredRegion.SetIndex( start );

	filter->SetExtractionRegion( desiredRegion );

  	filter->SetInput( reader->GetOutput() );
  	writer->SetInput( filter->GetOutput() );

	try{
		writer->Update();
	} catch (itk::ExceptionObject &err) {
		std::cerr << "ExceptionObject caught" << std::endl;
		std::cerr << err << std::endl;
	}
}

//Creating the input file name for a nifti
std::string makeInputFileName (const std::string &filename, const std::string &inputType){
	std::string inputFileName = "../data/";
	inputFileName.append(filename);
	inputFileName.append(inputType);
	return inputFileName;
}


std::string makeOutputFileName (const std::string &filename, const std::string &filetype){
	std::string OutputFileName = "../output/";
	OutputFileName.append(filename);
	OutputFileName.append("_HistogramFilterMid");
	OutputFileName.append(filetype);
	return OutputFileName;
}

template <typename T> std::string returnPointString(const T &number){
    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << number;
    std::string s = stream.str();

    s.replace(s.find('.'), 1, "p");
    return s;
}

void extract2DNormal (const std::string &filename, const std::string &filetype){
	using InputPixelType = float;
	using OutputPixelType = float;

	using InputImageType = itk::Image< InputPixelType, 3 >;
	using OutputImageType = itk::Image< OutputPixelType, 2>;

	using ReaderType = itk::ImageFileReader< InputImageType >;
	using WriterType = itk::ImageFileWriter< OutputImageType >;

	std::string inputFileName = makeInputFileName(filename, filetype);
	std::string outputName = "../output/";
	outputName.append(filename);
	outputName.append("_ChosenMiddleSlice").append(".tif");

	ReaderType::Pointer reader = ReaderType::New();
	WriterType::Pointer writer = WriterType::New();


	reader->SetFileName( inputFileName );
	writer->SetFileName( outputName );
	reader->Update();

	using FilterType = itk::ExtractImageFilter< InputImageType, OutputImageType > ;
	FilterType::Pointer filter = FilterType::New();
	filter->InPlaceOn();
	filter->SetDirectionCollapseToSubmatrix();

	InputImageType::RegionType inputRegion = reader->GetOutput()->GetLargestPossibleRegion();

    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << number;
    std::string s = stream.str();

    s.replace(s.find('.'), 1, "p");
    return s;
}

void extract2DNormal (const std::string &filename, const std::string &filetype){
	using InputPixelType = float;
	using OutputPixelType = float;

	using InputImageType = itk::Image< InputPixelType, 3 >;
	using OutputImageType = itk::Image< OutputPixelType, 2>;

	using ReaderType = itk::ImageFileReader< InputImageType >;
	using WriterType = itk::ImageFileWriter< OutputImageType >;

	std::string inputFileName = makeInputFileName(filename, filetype);
	std::string outputName = "../output/";
	outputName.append(filename);
	outputName.append("_ChosenMiddleSlice").append(".tif");

	ReaderType::Pointer reader = ReaderType::New();
	WriterType::Pointer writer = WriterType::New();


	reader->SetFileName( inputFileName );
	writer->SetFileName( outputName );
	reader->Update();

	using FilterType = itk::ExtractImageFilter< InputImageType, OutputImageType > ;
	FilterType::Pointer filter = FilterType::New();
	filter->InPlaceOn();
	filter->SetDirectionCollapseToSubmatrix();

	InputImageType::RegionType inputRegion = reader->GetOutput()->GetLargestPossibleRegion();

	InputImageType::SizeType size = inputRegion.GetSize();
	size[0] = 0;
	std::cout << size << std::endl;

	InputImageType::IndexType start = inputRegion.GetIndex();
	const unsigned int sliceNumber = 250;
	start[0] = sliceNumber;
	std::cout << start << std::endl;

	InputImageType::RegionType desiredRegion;
	desiredRegion.SetSize( size );
	desiredRegion.SetIndex( start );

	filter->SetExtractionRegion( desiredRegion );

  	filter->SetInput( reader->GetOutput() );
  	writer->SetInput( filter->GetOutput() );

	try{
		writer->Update();
	} catch (itk::ExceptionObject &err) {
		std::cerr << "ExceptionObject caught" << std::endl;
		std::cerr << err << std::endl;
	}

}


////////////////////////////////Previous 2D Slicer
//	using InputPixelType = float;
//	using OutputPixelType = float;
//
//	using InputImageType = itk::Image< InputPixelType, 3 >;
//	using OutputImageType = itk::Image< OutputPixelType, 2>;
//
//	using ReaderType = itk::ImageFileReader< InputImageType >;
//	using WriterType = itk::ImageFileWriter< OutputImageType >;
//
//	std::string inputFileName = makeInputFileName(filename, filetype);
//	std::string outputName = "../output/";
//	outputName.append(filename);
//	outputName.append("_ChosenMiddleSlice").append(".tif");
//
//	ReaderType::Pointer reader = ReaderType::New();
//	WriterType::Pointer writer = WriterType::New();
//
//
//	reader->SetFileName( inputFileName );
//	writer->SetFileName( outputName );
//	reader->Update();
//
//	using FilterType = itk::ExtractImageFilter< InputImageType, OutputImageType > ;
//	FilterType::Pointer filter = FilterType::New();
//	filter->InPlaceOn();
//	filter->SetDirectionCollapseToSubmatrix();
//
//	InputImageType::RegionType inputRegion = reader->GetOutput()->GetLargestPossibleRegion();
//
//	InputImageType::SizeType size = inputRegion.GetSize();
//	size[0] = 0;
//	std::cout << size << std::endl;
//
//	InputImageType::IndexType start = inputRegion.GetIndex();
//	const unsigned int sliceNumber = 250;
//	start[0] = sliceNumber;
//	std::cout << start << std::endl;
//
//	InputImageType::RegionType desiredRegion;
//	desiredRegion.SetSize( size );
//	desiredRegion.SetIndex( start );
//
//	filter->SetExtractionRegion( desiredRegion );
//
//  	filter->SetInput( reader->GetOutput() );
//  	writer->SetInput( filter->GetOutput() );
//
//	try{
//		writer->Update();
//	} catch (itk::ExceptionObject &err) {
//		std::cerr << "ExceptionObject caught" << std::endl;
//		std::cerr << err << std::endl;
//	}





//////////////////////THE PREVIOUS MAIN
	//std::cout << "Starting histogram filter on slices"  << std::endl;
	//
	//if (argc > 5){
	//	std::cout << "too many arguments" << std::endl;
	//	return EXIT_FAILURE;
	//}

	//// setting up arguments
	//std::string filename, inputType, outputType;
	//int orientation, scaleToUsual;
	//
	//// constexpr, computation at compile time
	//constexpr unsigned int Dimension = 3;
	//constexpr float desiredMinimum = 0.0;
	//constexpr float desiredMaximum = 255.0;
	//
	//if (argc == 5){
	//	std::cout << "Accepted input arguments" << std::endl;
	//	filename = argv[0];
	//	inputType = argv[1];
	//	outputType = argv[2];
	//	orientation = argv[3];
	//	scaleToUsual = argv[4];
	//} else {
	//	std::cout << "Not enough arguments, went with default" << std::endl;
	//	filename = "Smallfield_OCT_Angiography_Volume_fovea"; //filename in data/
	//	inputType = ".nii";
	//	outputType = inputType;
	//	orientation = 0;
	//	scaleToUsual = 0;

	//}

	////timing
	//auto start = std::chrono::high_resolution_clock::now();	

	//std::string inputFileName = makeInputFileName(filename, filetype);
	//
	//auto stop = std::chrono::high_resolution_clock::now();
	//auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	//std::cout << duration.count() << " milliseconds for reading in the file and creating constants"<<std::endl;


  	//
	//std::string outputFileName = makeOutputFileName(filename, filetype, alpha, beta, gamma, sigmaMinimum, sigmaMaximum, numberOfSigmaSteps);


	////Setting up the image reader of the particular type
	//using PixelType = float;
  	//using ImageType = itk::Image< PixelType, Dimension >;
  	//using ReaderType = itk::ImageFileReader< ImageType >;

	////Setting up the reader
  	//ReaderType::Pointer reader = ReaderType::New();
  	//reader->SetFileName( inputFileName );
  	//reader->Update();

 	//stop = std::chrono::high_resolution_clock::now();
	//duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	//std::cout << duration.count() << " milliseconds for setting image and reader"<<std::endl;

	//ImageType::Pointer image = reader->GetOutput();
	//ImageType::RegionType region = image->GetLargestPossibleRegion();
	//ImageType::SizeType size = region.GetSize();
	//std::cout << size << std::endl;


  	//using HessianPixelType = itk::SymmetricSecondRankTensor< double, Dimension >;
  	//using HessianImageType = itk::Image< HessianPixelType, Dimension >;
	//std::cout << duration.count() << " milliseconds for setting min max intensity"<< std::endl;

	////Setting up the output writer
  	//using WriterType = itk::ImageFileWriter< OutputImageType >;//setting up type for writer
  	//WriterType::Pointer writer = WriterType::New();//initialize new writer pointer
  	//writer->SetFileName( outputFileName );//set filename for writer
  	//writer->SetInput( rescaleFilter->GetOutput() );//
	//
	//stop = std::chrono::high_resolution_clock::now();
	//duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	//std::cout << duration.count() << " milliseconds for setting up writer" << std::endl;


	////extract 2d
	//extract2DNormal (filename,filetype);
  	//stop = std::chrono::high_resolution_clock::now();
	//duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	//std::cout << duration.count() << " milliseconds for extracting2D" << std::endl;


	////Write to file
  	//try {
    	//writer->Update();
    	//} catch ( itk::ExceptionObject & error ){
    	//std::cerr << "Error: " << error << std::endl;
    	//return EXIT_FAILURE;
    	//}
	//
	//stop = std::chrono::high_resolution_clock::now();
	//duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
	//std::cout << duration.count() << " seconds when file written out succesfully" << std::endl;
  	//return EXIT_SUCCESS;
	
