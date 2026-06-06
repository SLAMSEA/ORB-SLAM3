/**
 * This file is part of ORB-SLAM3
 *
 * Copyright (C) 2017-2020 Carlos Campos, Richard Elvira, Juan D. Gómez
 * Rodríguez, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
 * Copyright (C) 2014-2016 Raúl Mur-Artal, José M.M. Montiel and Juan D. Tardós,
 * University of Zaragoza.
 *
 * ORB-SLAM3 is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * ORB-SLAM3 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ORB-SLAM3. If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <unistd.h>

#include <opencv2/core/core.hpp>

#include <System.h>

using namespace std;

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        cerr << endl
             << "Usage: ./mono_gopro path_to_vocabulary "
             << "path_to_settings path_to_gopro_video"
             << endl;

        return 1;
    }

    // Open settings to get image resolution
    cv::FileStorage fsSettings(argv[2], cv::FileStorage::READ);

    if (!fsSettings.isOpened())
    {
        cerr << "Failed to open settings file at: "
             << argv[2] << endl;

        exit(-1);
    }

    cv::Size img_size(
        fsSettings["Camera.width"],
        fsSettings["Camera.height"]);

    fsSettings.release();

    // Create SLAM system
    ORB_SLAM3::System SLAM(
        argv[1],
        argv[2],
        ORB_SLAM3::System::MONOCULAR,
        true);

    // Vector for tracking time statistics
    vector<float> vTimesTrack;

    // Open video
    cv::VideoCapture cap(argv[3]);

    if (!cap.isOpened())
    {
        cout << "Error opening video stream or file"
             << endl;

        return -1;
    }

    int img_id = 0;

    int nImages =
        static_cast<int>(
            cap.get(cv::CAP_PROP_FRAME_COUNT));

    double fps =
        cap.get(cv::CAP_PROP_FPS);

    double frame_diff_s = 1.0 / fps;

    // Main loop
    while (true)
    {
        cv::Mat im, im_track;

        bool success = cap.read(im);

        // End of video
        if (!success || im.empty())
        {
            cout << "End of video." << endl;
            break;
        }

        im_track = im.clone();

        ++img_id;

        // Stable timestamp
        double tframe =
            static_cast<double>(img_id) / fps;

        // Resize image
        cv::resize(im_track, im_track, img_size);

#ifdef COMPILEDWITHC11
        chrono::steady_clock::time_point t1 =
            chrono::steady_clock::now();
#else
        chrono::monotonic_clock::time_point t1 =
            chrono::monotonic_clock::now();
#endif

        // Pass image to SLAM
        SLAM.TrackMonocular(im_track, tframe);

#ifdef COMPILEDWITHC11
        chrono::steady_clock::time_point t2 =
            chrono::steady_clock::now();
#else
        chrono::monotonic_clock::time_point t2 =
            chrono::monotonic_clock::now();
#endif

        double ttrack =
            chrono::duration_cast<
                chrono::duration<double>>(
                t2 - t1)
                .count();

        if (img_id % 100 == 0)
        {
            cout << "Video FPS: "
                 << fps << "\n";

            cout << "ORB-SLAM3 running at: "
                 << 1.0 / ttrack
                 << " FPS\n";
        }
        // Autosave every 500 frames
        if (img_id % 150 == 0)
        {
            cout << "Autosaving trajectory..."
                << endl;

            SLAM.SaveKeyFrameTrajectoryTUM(
                "KeyFrameTrajectory_autosave.txt");

            cout << "Autosave complete."
                << endl;
        }

        vTimesTrack.push_back(ttrack);

        // Wait to simulate real-time playback
        if (ttrack < frame_diff_s)
        {
            usleep(
                (frame_diff_s - ttrack) * 1e6);
        }
    }

    // Shutdown SLAM properly
    SLAM.Shutdown();

    // Give viewer / local mapping time to finish
    usleep(1000000);

    // Tracking time statistics
    sort(
        vTimesTrack.begin(),
        vTimesTrack.end());

    float totaltime = 0;

    for (size_t i = 0;
         i < vTimesTrack.size();
         i++)
    {
        totaltime += vTimesTrack[i];
    }

    cout << "-------" << endl
         << endl;

    if (!vTimesTrack.empty())
    {
        cout << "median tracking time: "
             << vTimesTrack[
                    vTimesTrack.size() / 2]
             << endl;

        cout << "mean tracking time: "
             << totaltime /
                    vTimesTrack.size()
             << endl;
    }

    // Save trajectories
    cout << "Saving trajectories..."
         << endl;

    SLAM.SaveKeyFrameTrajectoryTUM(
        "KeyFrameTrajectory.txt");

    cout << "Trajectories saved."
         << endl;

    return 0;
}
