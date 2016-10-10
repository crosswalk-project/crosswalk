// Copyright 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.xwalk.core.XWalkFindListener;

/**
 * Base class for XWalkView find-in-page API tests.
 */
public class XWalkViewFindApisTestBase extends XWalkViewTestBase {

    // The  interface to intercept find results from XWalkView
    private XWalkFindListener mXWalkFindResultListener;

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        setFindListener();
    }

    class TestXWalkFindListener extends XWalkFindListener {
        @Override
        public void onFindResultReceived(int activeMatchOrdinal, int numberOfMatches,
                boolean isDoneCounting) {
            mXWalkFindResultListener.onFindResultReceived(activeMatchOrdinal, numberOfMatches,
                    isDoneCounting);
        }
    }

    void setFindListener() {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().setFindListener(new TestXWalkFindListener());
            }
        });
    }

    /**
     * Invokes findAllAsync on the UI thread, blocks until find results are
     * received, and returns the number of matches.
     *
     * @param searchString A string to search for.
     * @return The number of instances of the string that were found.
     * @throws Throwable
     */
    protected int findAllAsyncOnUiThread(final String searchString)
            throws Throwable {
        final IntegerFuture future = new IntegerFuture() {
            @Override
            public void run() {
                mXWalkFindResultListener = new XWalkFindListener() {
                    @Override
                    public void onFindResultReceived(int activeMatchOrdinal, int numberOfMatches,
                            boolean isDoneCounting) {
                        if (isDoneCounting) set(numberOfMatches);
                    }
                };
                getXWalkView().findAllAsync(searchString);
            }
        };
        runTestOnUiThread(future);
        return future.get(10, TimeUnit.SECONDS);
    }

    /**
     * Invokes findNext on the UI thread, blocks until find results are
     * received, and returns the ordinal of the highlighted match.
     *
     * @param forwards The direction to search as a boolean, with forwards
     *                 represented as true and backwards as false.
     * @return The ordinal of the highlighted match.
     * @throws Throwable
     */
    protected int findNextOnUiThread(final boolean forwards)
            throws Throwable {
        final IntegerFuture future = new IntegerFuture() {
            @Override
            public void run() {
                mXWalkFindResultListener = new XWalkFindListener() {
                    @Override
                    public void onFindResultReceived(int activeMatchOrdinal, int numberOfMatches,
                            boolean isDoneCounting) {
                        if (isDoneCounting) set(activeMatchOrdinal);
                    }
                };
                getXWalkView().findNext(forwards);
            }
        };
        runTestOnUiThread(future);
        return future.get(10, TimeUnit.SECONDS);
    }

    /**
     * Invokes clearMatches on the UI thread.
     *
     * @throws Throwable
     */
    protected void clearMatchesOnUiThread() throws Throwable {
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                getXWalkView().clearMatches();
            }
        });
    }

    // Similar to java.util.concurrent.Future, but without the ability to cancel.
    private abstract static class IntegerFuture implements Runnable {
        private CountDownLatch mLatch = new CountDownLatch(1);
        private int mValue;

        @Override
        public abstract void run();

        /**
         * Gets the value of this Future, blocking for up to the specified
         * timeout for it become available. Throws a TimeoutException if the
         * timeout expires.
         */
        public int get(long timeout, TimeUnit unit) throws Throwable {
            if (!mLatch.await(timeout, unit)) {
                throw new TimeoutException();
            }
            return mValue;
        }

        /**
         * Sets the value of this Future.
         */
        protected void set(int value) {
            mValue = value;
            mLatch.countDown();
        }
    }
}

