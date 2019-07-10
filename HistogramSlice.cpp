// File name: 	HistogramSlice.cpp
// Author: 	Viet Than
// Email: 	viet.than@vanderbilt.edu (thanhoangviet@gmail.com)
// Lab: 	Medical Imaging Lab under Ipek Oguz at Vanderbilt University, TN, USA
// Description: Histogram normalize a volume slice by slice with reference to the slice middle,
// 		output differently depend on direction.



#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkExtractImageFilter.h"
#include "itkPasteImageFilter.h"

#include "itkHistogramMatchingImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

#include <string>
#include <iostream>
#include <chrono>

using namespace itk;

//helper functions
std::string makeInputFileName (const std::string &filename, const std::string &filetype);
std::string makeOutputFileName (const std::string &filename, const std::string &filetype);



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
		orientation = atoi(argv[4]);
		scaleToUsual = atoi(argv[5]);
	} else {
		std::cout << "Not enough arguments, went with default" << std::endl;
		filename = "Smallfield_OCT_Angiography_Volume_fovea"; //filename in data/
		inputType = ".nii";
		outputType = inputType;
		orientation = 0;
		scaleToUsual = 0;

	}
	//timing
	auto begin = std::chrono::high_resolution_clock::now();	

	std::string inputFileName = makeInputFileName(filename, inputType);	// input is assumed in ../data/
	std::string outputFileName = makeOutputFileName(filename, outputType);
	
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - begin);
	std::cout << duration.count() << " milliseconds for reading in the file and creating constants"<<std::endl;
	
	// setting up reader type
	using imagePixelType = short;						// short is faster
	using ImageType = itk::Image< imagePixelType, Dimension>;		// ImageType is used for both input and output
	using ReaderType = itk::ImageFileReader< ImageType >;
	
	// Setting up writer
	using WriterType = itk::ImageFileWriter< ImageType >;
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName( outputFileName );

	// setting up image reader
	ReaderType::Pointer imageReader = ReaderType::New();
	imageReader->SetFileName( inputFileName );
	
	// retrieve image with Update()
  	try{
    		imageReader->Update();
	} catch( itk::ExceptionObject & err ){
    		std::cerr << "ExceptionObject caught !" << std::endl;
    		std::cerr << err << std::endl;
    		return EXIT_FAILURE;
    	}

	// get image specifications for use
	ImageType * inputImage = imageReader->GetOutput();				// get the input image
	ImageType::RegionType inputRegion = inputImage->GetLargestPossibleRegion();	// get image region
	ImageType::SizeType size = inputRegion.GetSize();				//getting the region size
	ImageType::IndexType start = inputRegion.GetIndex();				// get index start
	
	// set size of slice
	ImageType::SizeType sliceSize = size;						// set up the size for any slice
	sliceSize[orientation] = 1; 							// setting the orientation direction to 1 for collapse

	using ExtractFilterType = itk::ExtractImageFilter< ImageType, ImageType >; 	// output 3d slices (not 2d slices)
	
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - begin);
	std::cout << duration.count() << " milliseconds to set up all reader writer extraction filter" << std::endl;


	/********** MIDDLE SLICE EXTRACTION **********/

	// set the index need to extract middle slice
	const unsigned int midSliceNumber = (int) size[orientation]/2;			// finding middle slice
	ImageType::IndexType middleStart = start;					// create a index for middle slice
	middleStart[orientation] = midSliceNumber;					// pick the middle slice index
	
	// set up extraction region for middle slice
	ImageType::RegionType desiredMiddleRegion; 					// create middle region
	desiredMiddleRegion.SetSize( sliceSize ); 					// set size (with collapsed direction) for middle region
	desiredMiddleRegion.SetIndex( middleStart ); 					// set extraction region index start
	std::cout << sliceSize << std::endl;
	std::cout << middleStart << std::endl;
	// setting up Extraction filter for middle slice
	ExtractFilterType::Pointer extractMiddleFilter = ExtractFilterType::New();	// make filter to extract middle slice
	extractMiddleFilter->SetDirectionCollapseToSubmatrix();				// set flag to collapse
	extractMiddleFilter->SetExtractionRegion( desiredMiddleRegion );		// set region
	extractMiddleFilter->SetInput( inputImage );					// set input image
	extractMiddleFilter->Update();
	ImageType::Pointer middleSlice = extractMiddleFilter->GetOutput();		// get middle slice for later use

	std::cout << "right after extraction\n";
	std::cout << middleSlice->GetLargestPossibleRegion() << "\n";
	
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - begin);
	std::cout << duration.count() << " milliseconds to extract middle slice" << std::endl;
	
	/********** SLICE EXTRACTION AND HISTOGRAM MATCH FILTER **********/
	

	// setting up HistogramMatchFilter (HMFilter)
	using HMFilterType = itk::HistogramMatchingImageFilter< ImageType , ImageType >;
	HMFilterType::Pointer intensityEqualizeFilter = HMFilterType::New();
	intensityEqualizeFilter->SetNumberOfHistogramLevels(100);
	intensityEqualizeFilter->SetNumberOfMatchPoints(15);
	intensityEqualizeFilter->ThresholdAtMeanIntensityOn();
	intensityEqualizeFilter->SetReferenceImage( middleSlice );

	// Setting up Paste Filter
	ImageType * outputImage = inputImage;					// make outputImage
	using PasteFilterType = itk::PasteImageFilter< ImageType, ImageType >;
	PasteFilterType::Pointer pasteFilter = PasteFilterType::New();
	pasteFilter->SetSourceImage( outputImage );

	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - begin);
	std::cout << duration.count() << " milliseconds to set up histogram and paste filter" << std::endl;



	for (int i = 1; i <= size[orientation]; ++i){
		/*** SLICE EXTRACTION ***/
		// set the index need to extract a slice
		ImageType::IndexType sliceStart = start; std::cout<< 1 << "\n";				// create a index
		sliceStart[orientation] = i;std::cout<< 2 << "\n";							// pick the index
	
		// set up extraction region for a slice
		ImageType::RegionType desiredSliceRegion; std::cout<< 3 << "\n";					// create slice region
		desiredSliceRegion.SetSize( sliceSize ); std::cout<< 4 << "\n";					// set size (with collapsed direction) 
		desiredSliceRegion.SetIndex( sliceStart ); std::cout<< 5 << "\n";					// set extraction region index start
		std::cout << "slice number = " << i << " \n";

		// setting up Extraction filter for middle slice
		ExtractFilterType::Pointer extractSliceFilter = ExtractFilterType::New();std::cout<< 6 << "\n";	// make filter to extract a slice
		extractSliceFilter->SetDirectionCollapseToSubmatrix();std::cout<< 7 << "\n";				// set flag to collapse
		extractSliceFilter->SetExtractionRegion( desiredSliceRegion );	std::cout<< 8 << "\n";		// set region
		extractSliceFilter->SetInput( inputImage );std::cout<< 9 << "\n";					// set input image
		extractSliceFilter->Update();
		ImageType::Pointer currentSlice = extractSliceFilter->GetOutput();std::cout<< 10 << "\n";		// get slice for later use

		// update HMFilter
		intensityEqualizeFilter->SetInput( currentSlice );std::cout<< 11 << "\n";
		std::cout << currentSlice->GetLargestPossibleRegion() << "\n";
		std::cout << middleSlice->GetLargestPossibleRegion() << "\n";
		intensityEqualizeFilter->Update();std::cout<< 12 << "\n";

		// use pasteFilter
		pasteFilter->SetDestinationImage( intensityEqualizeFilter->GetOutput() );std::cout<< 13 << "\n";
		pasteFilter->SetDestinationIndex( sliceStart );std::cout<< 14 << "\n";
		pasteFilter->SetSourceRegion( inputRegion );std::cout<< 15 << "\n";
		pasteFilter->Update();std::cout<< 16 << "\n";
	}










	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - begin);
	std::cout << duration.count() << " about to use writer" << std::endl;


  	writer->SetInput( pasteFilter->GetOutput() );

	try{
		writer->Update();
	} catch (itk::ExceptionObject &err) {
		std::cerr << "ExceptionObject caught" << std::endl;
		std::cerr << err << std::endl;
	}

	return EXIT_SUCCESS;
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
	
