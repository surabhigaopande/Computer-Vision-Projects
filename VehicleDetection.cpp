#include <iostream>
#include <math.h>
#include <stdio.h>
#include <string>

// OpenCV includes.
#include "cv.h"
#include "highgui.h"

#define DEBUG 1

// Set up the AVI Writer object
typedef struct CvAVIWriter CvAVIWriter;

int main(int argc, char* argv[])
{
 	// STEP 1:
	// Bring the video file (AVI) in. Either a stored file or live from Camera using cvCaptureFromCAM
	CvCapture* VideoFile = cvCaptureFromFile("clip.avi");
	if (VideoFile == NULL)
	{
		std::cout << "Uh-oh.  Either the input file doesn't exist, or OpenCV cannot read it." << std::endl;
		return 1;
	}

	// Now let's set up the frame size so that we can vomit out a video...
	CvSize frame_size;
	frame_size.height = cvGetCaptureProperty(VideoFile,CV_CAP_PROP_FRAME_HEIGHT);
	frame_size.width  = cvGetCaptureProperty(VideoFile, CV_CAP_PROP_FRAME_WIDTH);
	// We'll go ahead and say that the AVI file is loaded now:
	if(DEBUG)
	{std::cout << "Brought in AVI file." << std::endl;}

	// Figure out what our incoming movie file looks like
	double FPS = cvGetCaptureProperty(VideoFile, CV_CAP_PROP_FPS);
	double FOURCC = cvGetCaptureProperty(VideoFile, CV_CAP_PROP_FOURCC);
	if(DEBUG)
	{
		std::cout << "FPS:  " << FPS << std::endl;
		std::cout << "FOURCC:  " << FOURCC << std::endl;
	}

	// Create a CvVideoWriter.  The arguments are the name of the output file (must be .avi),
	// a macro for a four-character video codec installed on your system, the desired frame
	// rate of the video, and the video dimensions.
	CvVideoWriter* videoWriter = cvCreateVideoWriter("sidewalk_output.avi",CV_FOURCC('D', 'I', 'V', 'X'), FPS, cvSize(frame_size.width, frame_size.height));
	// Now we can say that the VideoWriter is created:
	if(DEBUG)
	{std::cout << "videoWriter is made." << std::endl;}

	// Make display windows
	cvNamedWindow("background Frame", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("current Frame", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("diff Frame", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("output Frame", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("ROI Frame", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("ROI Frame (Color)", CV_WINDOW_AUTOSIZE);

	// Keep track of frames
	static int imageCount = 0;

	// Set up images.
	IplImage* diffFrame = cvCreateImage(cvSize(frame_size.width, frame_size.height), IPL_DEPTH_8U, 1);
	IplImage* backgroundFrame, *eig_image, *temp_image;
	IplImage* currentFrame = cvCreateImage(cvSize(frame_size.width, frame_size.height), IPL_DEPTH_8U, 1);
	IplImage* outFrame = cvCreateImage(cvSize(frame_size.width, frame_size.height), IPL_DEPTH_8U, 3);
	IplImage* tempFrameBGR = cvCreateImage(cvSize(frame_size.width, frame_size.height), IPL_DEPTH_8U, 3);
	IplImage* ROIFrame = cvCreateImage(cvSize((265-72), (214-148)), IPL_DEPTH_8U, 1);
	IplImage* ROIFrame2 = cvCreateImage(cvSize((265-72), (214-148)), IPL_DEPTH_8U, 1);
	IplImage* ROIFrameBGR = cvCreateImage(cvSize((265-72), (214-148)), IPL_DEPTH_8U, 3);
	IplImage* ROIFrameBGRPrior = cvCreateImage(cvSize((265-72), (214-148)), IPL_DEPTH_8U, 3);

	// And now set up the data for MinMaxLoc (for ROI image)
	double minVal, maxVal;
	CvPoint minLoc, maxLoc, outPoint;

	// Initialize our contour information...
	int contours=0;
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq** firstContour;
	int headerSize;
	CvSeq* contour = 0;
	int color = (0, 0, 0);
	CvContourScanner ContourScanner=0;

	// Zero out the vehicles-counting image...
	cvZero(ROIFrameBGR);

	int vehicles=0;
	int MOVEMENT=0;

	// There's gotta be a better way to do this... like with threading?
	while(1)
	{

		IplImage* tempFrame = cvQueryFrame(VideoFile);
		// If the video HAS a current frame...
		if (tempFrame != NULL)
		{
			// The video is BGR-space.  I wish there were a cvGetColorSpace command or something...
			cvCvtColor(tempFrame, currentFrame, CV_BGR2GRAY);
			// Grrr ... flipped.

			// Get initial "background" image...
			if (imageCount==0)
			{
				//IplImage* backgroundFrame = cvCloneImage(currentFrame);
				backgroundFrame = cvCloneImage(currentFrame);
			}
			cvShowImage("background Frame", backgroundFrame);
			cvShowImage("current Frame", currentFrame);
			cvAbsDiff(currentFrame,backgroundFrame,diffFrame);
			//if(DEBUG)
			//{std::cout << "Pulled in video grab of frame " << imageCount << "." << std::endl;}

			// Back to color ...
			cvCvtColor(diffFrame, outFrame, CV_GRAY2BGR);
			// Now let's go ahead and put up a box (rect, actually) for our ROI.
			// (72, 148)+-----------------------+(265, 148)
			//			|						|
			// (72, 214)+-----------------------+(265, 214)
			//MotionRegion cvRect(72, 148, (265-72), (214-148));
			//cvRectangle(outFrame, cvPoint(72, 148), cvPoint(265, 214), CV_RGB(255, 0, 255), 1);
			cvRectangle(diffFrame, cvPoint(100, 100), cvPoint(120, 250), CV_RGB(255, 0, 255), 1);
			cvShowImage("diff Frame", diffFrame);
			cvCvtColor(backgroundFrame, tempFrameBGR, CV_GRAY2BGR);

			// ROIFrame is BW.
			ROIFrame = cvCloneImage(outFrame);
			cvSetImageROI(ROIFrame, cvRect(100, 100, (265-72), (250-100)));
			//cvOr(outFrame, tempFrame, outFrame);
			cvShowImage("ROI Frame", ROIFrame);
			// Great.  The ROI Frame works, almost as an "inset".
			// Now let's find when motion exists within the ROI.
			// First:  the cumbersome way...

			cvSetImageCOI(ROIFrame, 1);
			cvMinMaxLoc(ROIFrame, &minVal, &maxVal, &minLoc, &maxLoc, NULL);
			if (maxVal <75 )
			{
				// Zero out the LAST vehicles-counting image...
				cvZero(ROIFrameBGRPrior);
				MOVEMENT=0;
			}
			if(maxVal > 75)
			{
				cvSetImageCOI(ROIFrameBGRPrior, 1);
				// We are starting a motion sequence...
				if( (MOVEMENT==0) && (cvCountNonZero(ROIFrameBGRPrior)==0) )
				{
					// Zero out the vehicles-counting image...
					cvZero(ROIFrameBGR);
					MOVEMENT=1;

					vehicles++;
					if(DEBUG)
					{std::cout << "ROI has counted " << vehicles << " vehicles." << std::endl;}
				}


				if(DEBUG)
				{std::cout << "We have motion in the ROI!  maxVal: " << maxVal << " minVal: " << minVal << std::endl;}
				// We can figure out when there's motion within the ROI.  Good.
				// Let's instead try to put a dot on vehicles that are moving...
				cvCircle(ROIFrameBGR, maxLoc, 1, CV_RGB(255, 0, 0), 1);
				ROIFrameBGRPrior = cvCloneImage(ROIFrameBGR);
			}
			cvShowImage("ROI Frame (Color)", ROIFrameBGR);
			// Build up the output ...
			cvOr(outFrame, tempFrame, outFrame);
			// ... and draw the ROI rectangle.
			cvRectangle(outFrame, cvPoint(100, 100), cvPoint(120, 250), CV_RGB(255, 0, 255), 1);
			char vehiclesCount[32];
			if (vehicles==1)
			{sprintf(vehiclesCount, "%d vehicle", vehicles);}
			else if ( (vehicles < 1) || (vehicles > 1) )
			{sprintf(vehiclesCount, "%d vehicles", vehicles);}
			CvFont font;
			cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.8, 0.8, 0, 2);
			cvPutText(outFrame, vehiclesCount, cvPoint(0, 25), &font, cvScalar(0, 0, 300));
			cvShowImage("output Frame", outFrame);

			cvWriteFrame(videoWriter, outFrame);
			//if(DEBUG)
			//{std::cout << "Wrote frame to output AVI file." << std::endl;}
			imageCount++;
 		}	// end if (image != NULL) loop

		// This will return the code of the pressed key or -1 if
		// nothing was pressed before 10 ms elapsed.
		int keyCode = cvWaitKey(33);
		if ( (keyCode == 's') || (keyCode == 'S') )
		{
			while(1)
			{
				keyCode = cvWaitKey(33);
				if ( (keyCode == 's') || (keyCode == 'S') )
				{
					keyCode = 999;
					break;
				}
			}
		}

		// But the video may have ended...
		if( (tempFrame == NULL) || (keyCode >= 0) && (keyCode != 999) )
		{
			// Either the video is over or a key was pressed.
			// Dump the video file.
			cvReleaseCapture(&VideoFile);
			// Release the videoWriter from memory.
			cvReleaseVideoWriter(&videoWriter);
			// Release images from memory...
			cvReleaseImage(&currentFrame);
			//cvReleaseImage(&diffFrame);
			// ... And destroy the windows.
			cvDestroyWindow("Video Frame");
			std::cout << "Released VideoFile and VideoWriter." << std::endl;
			return 0;
			exit(0);
		}
	}// end while loop
	return 0;
}
