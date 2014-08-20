/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "itkDCMTKTransformIO.h"
#include "itkDCMTKTransformIOFactory.h"
#include "itkTransformFileReader.h"
#include "itkImageSeriesReader.h"
#include "itkDCMTKImageIO.h"
#include "itkDCMTKSeriesFileNames.h"
#include "itkTestingMacros.h"
#include "itkCompositeTransform.h"

int itkDCMTKTransformIOTest(int argc, char* argv[])
{
  if( argc < 4 )
    {
    std::cerr << "Usage: " << argv[0]
              << " fixedSeriesDirectory movingSeriesDirectory transform" << std::endl;
    return EXIT_FAILURE;
    }
  const char * fixedSeriesDirectory = argv[1];
  const char * movingSeriesDirectory = argv[2];
  const char * transformFileName = argv[3];

  const unsigned int Dimension = 3;
  typedef short                              PixelType;
  typedef itk::Image< PixelType, Dimension > ImageType;

  typedef itk::ImageSeriesReader< ImageType > ReaderType;
  ReaderType::Pointer fixedReader = ReaderType::New();

  typedef itk::DCMTKImageIO ImageIOType;
  ImageIOType::Pointer fixedIO = ImageIOType::New();
  fixedReader->SetImageIO( fixedIO );

  typedef itk::DCMTKSeriesFileNames SeriesFileNamesType;
  SeriesFileNamesType::Pointer fixedSeriesFileNames = SeriesFileNamesType::New();
  fixedSeriesFileNames->SetInputDirectory( fixedSeriesDirectory );
  typedef SeriesFileNamesType::FilenamesContainer FileNamesContainerType;
  const FileNamesContainerType & fixedFileNames = fixedSeriesFileNames->GetInputFileNames();
  std::cout << "There are " << fixedFileNames.size() << " fixed image slices." << std::endl;
  std::cout << "First fixed images Series UID: " << fixedSeriesFileNames->GetSeriesUIDs()[0] << "\n" << std::endl;
  fixedReader->SetFileNames( fixedFileNames );

  ReaderType::Pointer movingReader = ReaderType::New();
  ImageIOType::Pointer movingIO = ImageIOType::New();
  movingReader->SetImageIO( movingIO );

  SeriesFileNamesType::Pointer movingSeriesFileNames = SeriesFileNamesType::New();
  movingSeriesFileNames->SetInputDirectory( movingSeriesDirectory );
  const FileNamesContainerType &  movingFileNames = movingSeriesFileNames->GetInputFileNames();
  std::cout << "There are " << movingFileNames.size() << " moving image slices." << std::endl;
  std::cout << "First moving images Series UID: " << movingSeriesFileNames->GetSeriesUIDs()[0] << "\n" << std::endl;
  movingReader->SetFileNames( movingFileNames );

  TRY_EXPECT_NO_EXCEPTION( fixedReader->Update() );
  TRY_EXPECT_NO_EXCEPTION( movingReader->Update() );

  typedef float ScalarType;

  itk::DCMTKTransformIOFactory::Pointer dcmtkTransformIOFactory = itk::DCMTKTransformIOFactory::New();
  EXERCISE_BASIC_OBJECT_METHODS( dcmtkTransformIOFactory, itk::DCMTKTransformIOFactory );
  std::cout << std::endl;
  itk::ObjectFactoryBase::RegisterFactory( dcmtkTransformIOFactory );

  typedef itk::TransformFileReaderTemplate< ScalarType > TransformReaderType;
  TransformReaderType::Pointer transformReader = TransformReaderType::New();
  transformReader->SetFileName( transformFileName );

  typedef itk::DCMTKTransformIO< ScalarType > TransformIOType;
  TransformIOType::Pointer transformIO = TransformIOType::New();
  EXERCISE_BASIC_OBJECT_METHODS( transformIO, TransformIOType );
  transformReader->SetTransformIO( transformIO );

  TEST_EXPECT_TRUE( ! transformIO->CanWriteFile( transformFileName ) );

  TEST_EXPECT_TRUE( ! transformIO->CanReadFile( "fileThatDoesNotExist.dcm" ) );

  TEST_EXPECT_TRUE( transformIO->CanReadFile( transformFileName ) );

  TRY_EXPECT_EXCEPTION( transformIO->Write() );

  TRY_EXPECT_NO_EXCEPTION( transformReader->Update() );

  typedef TransformReaderType::TransformListType TransformListType;
  TransformListType * transformList = transformReader->GetTransformList();

  typedef itk::CompositeTransform< ScalarType, Dimension > ReadTransformType;
  TransformListType::const_iterator transformIt = transformList->begin();
  ReadTransformType::Pointer readTransform = dynamic_cast< ReadTransformType * >( (*transformIt).GetPointer() );
  if( readTransform.IsNull() )
    {
    std::cerr << "Did not get the expected transform out." << std::endl;
    return EXIT_FAILURE;
    }
  readTransform->Print( std::cout );
  TEST_EXPECT_EQUAL( 3, transformIO->GetReadTransformList().size() );

  transformIO->SetFrameOfReferenceUID( "1.2.826.0.1.3680043.8.274.1.1.8323328.22085.1372783046.709932" );
  TRY_EXPECT_NO_EXCEPTION( transformReader->Update() );
  TEST_EXPECT_EQUAL( 2, transformIO->GetReadTransformList().size() );

  return EXIT_SUCCESS;
}
